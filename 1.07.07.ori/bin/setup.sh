#!/bin/bash

MAKE_PROCS=1
SETUPDIR=/opt/movidius
VERBOSE=yes

### Do not edit below this line
# Bash colors
NC='\033[0m' # No Color
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'

err_report() {
    echo -e "${RED}Installation failed. Error on line $1${NC}"
}

trap 'err_report $LINENO' ERR

# Exit if a command fails
set -e

APT_QUIET=-qq
DD_QUIET=status=none
PIP_QUIET=--quiet
GIT_QUIET=-q
STDOUT_QUIET='>/dev/null'
#STDERR_QUIET='2>/dev/null'
#STDOUTERR_QUIET='&>/dev/null'


if [ $VERBOSE == yes ]; then
	APT_QUIET=
	DD_QUIET=
	PIP_QUIET=
	GIT_QUIET=
	STDOUT_QUIET=
	STDERR_QUIET=
	STDOUTERR_QUIET=
fi

# If the executing user is not root, ask for sudo priviledges
SUDO_PREFIX=""
if [ $EUID != 0 ]; then
	SUDO_PREFIX="sudo"
	sudo -v
	
	# Keep-alive: update existing sudo time stamp if set, otherwise do nothing.
	while true; do sudo -n true; sleep 60; kill -0 "$$" || exit; done 2>/dev/null &
fi

# Find where this script is located. Store it in DIR
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
	DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
	SOURCE="$(readlink "$SOURCE")"
	[[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# Print stdout and stderr in screen and in logfile
d=$(date +%y-%m-%d-%H-%M)
logfile="setup_"$d".log"
exec &> >(tee -a "$DIR/$logfile")

echo "Movidius Neural Compute Toolkit Setup."

# Get installation location
SETUPDIR_ORIG=$SETUPDIR
read -e -p "Enter installation location (default: $SETUPDIR, press enter for default location): " SETUPDIR

SETUPDIR=${SETUPDIR:-$SETUPDIR_ORIG}

# Create setup directory
echo "Creating setup directory..."
mkdir -p "$SETUPDIR" &>/dev/null || $SUDO_PREFIX mkdir -p "$SETUPDIR"
$SUDO_PREFIX chown $(id -un):$(id -gn) "$SETUPDIR"
# Get absolute dir
SETUPDIR="$( cd "$SETUPDIR" && pwd )"

echo "Checking Ubuntu version..."
os_distro=$(lsb_release -i 2>/dev/null | cut -f 2)
os_version=$(lsb_release -r 2>/dev/null | awk '{ print $2 }' | sed 's/[.]//')
if [ "${os_distro,,}" != "ubuntu" ] || [ 1604 != $os_version ]; then
	echo "Your current combination of linux distribution and distribution version is not officially supported! Continue at your own risk!"
fi

# Install apt dependencies
echo "Installing apt dependencies..."
$SUDO_PREFIX apt-get $APT_QUIET update
$SUDO_PREFIX apt-get $APT_QUIET install -y unzip coreutils curl git python3 python3-pip
$SUDO_PREFIX apt-get $APT_QUIET install -y $(cat "$DIR/requirements_apt.txt")
$SUDO_PREFIX apt-get $APT_QUIET install -y --no-install-recommends libboost-all-dev
$SUDO_PREFIX ldconfig

# Install python pip packages in userspace
echo "Installing python packages..."
$SUDO_PREFIX pip3 install $PIP_QUIET -I -r "$DIR/requirements.txt"

# Caffe
echo "Downloading Caffe..."
cd "$SETUPDIR"
if [ -d "caffe" ]; then
	cd caffe
	is_caffe_dir=`git remote show origin 2>&1 | grep -c -i "github.com/BVLC/caffe.git"`
	if [ "$is_caffe_dir" -gt 0 ]; then
		eval git reset --hard HEAD $STDOUT_QUIET
		eval git checkout $GIT_QUIET master $STDOUT_QUIET
		eval git branch -D fathom_branch &>/dev/null || true
		eval git pull $STDOUT_QUIET
	else
		echo "The directory \"$SETUPDIR/caffe\" is not the BVLC caffe project. Please remove/rename this directory and start over."
		exit 128
	fi
else
    eval git clone $GIT_QUIET https://github.com/BVLC/caffe.git $STDOUT_QUIET
fi

echo "Compiling Caffe..."
cd "$SETUPDIR"
export PYTHONPATH=$env:"$SETUPDIR/caffe/python":$PYTHONPATH
cd caffe
eval git checkout $GIT_QUIET 1.0 -b fathom_branch $STDOUT_QUIET
sed -i 's/python_version "2"/python_version "3"/' CMakeLists.txt
sed -i 's/CPU_ONLY  "Build Caffe without CUDA support" OFF/CPU_ONLY  "Build Caffe without CUDA support" ON/' CMakeLists.txt
mkdir -p build
cd build
eval cmake .. $STDOUT_QUIET
eval make -j $MAKE_PROCS all $STDOUT_QUIET

echo "Installing Caffe..."
eval make install $STDOUT_QUIET
# You can use 'make runtest' to test this stage manually :)

# USB Setup
echo "Configuring device udev rules"
$SUDO_PREFIX dd of=/etc/udev/rules.d/97-usbboot.rules $DD_QUIET <<EOF
SUBSYSTEM=="usb", ATTRS{idProduct}=="2150", ATTRS{idVendor}=="03e7", GROUP="users", MODE="0666", ENV{ID_MM_DEVICE_IGNORE}="1"
SUBSYSTEM=="usb", ATTRS{idProduct}=="f63b", ATTRS{idVendor}=="040e", GROUP="users", MODE="0666", ENV{ID_MM_DEVICE_IGNORE}="1"
SUBSYSTEM=="tty", ATTRS{idProduct}=="2150", ATTRS{idVendor}=="03e7", GROUP="users", MODE="0666", ENV{ID_MM_DEVICE_IGNORE}="1"
EOF
$SUDO_PREFIX udevadm control --reload-rules
$SUDO_PREFIX udevadm trigger

echo "Adding user '$USER' to 'users' group"
$SUDO_PREFIX usermod -a -G users $USER

# Add PYTHONPATH if not already there
echo "Augmenting PYTHONPATH environment variable"
grep -q -F "export PYTHONPATH=\$env:\"$SETUPDIR/caffe/python\":\$PYTHONPATH" $HOME/.bashrc || echo "export PYTHONPATH=\$env:\"$SETUPDIR/caffe/python\":\$PYTHONPATH" >> $HOME/.bashrc

echo -e "${GREEN}Setup is complete.${NC}
The PYTHONPATH enviroment variable was added to your .bashrc as described in the Caffe documentation. 
${YELLOW}Keep in mind that only newly spawned terminals can see this variable!
This means that you need to open a new terminal in order to be able to use the toolkit.${NC}"

echo "Please provide feedback in our support forum if you encountered difficulties."
