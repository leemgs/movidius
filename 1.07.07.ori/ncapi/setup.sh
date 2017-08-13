
sudo dpkg -i redist/*.deb
 
cd tools

./get_models.sh
./convert_models.sh

cd ..

