CAFFE_ROOT='/opt/movidius/caffe'

# Please Note, the NC API currently uses fixed up versions of prototxt. In future versions these fixups may not be needed.

#download googlenet
#wget -P ../networks/GoogLeNet/ -N https://raw.githubusercontent.com/BVLC/caffe/master/models/bvlc_googlenet/deploy.prototxt
wget -P ../networks/GoogLeNet/ -N http://dl.caffe.berkeleyvision.org/bvlc_googlenet.caffemodel
#download Alexnet:
#wget -P ../networks/AlexNet/ -N https://raw.githubusercontent.com/BVLC/caffe/master/models/bvlc_alexnet/deploy.prototxt
wget -P ../networks/AlexNet/ -N http://dl.caffe.berkeleyvision.org/bvlc_alexnet.caffemodel
#download squeezenet:
#wget -P ../networks/SqueezeNet/ -N https://raw.githubusercontent.com/DeepScale/SqueezeNet/master/SqueezeNet_v1.0/deploy.prototxt
wget -P ../networks/SqueezeNet/ -N https://github.com/DeepScale/SqueezeNet/raw/master/SqueezeNet_v1.0/squeezenet_v1.0.caffemodel
#download imagenet mean and word descriptions
sh $CAFFE_ROOT/data/ilsvrc12/get_ilsvrc_aux.sh
cp $CAFFE_ROOT/data/ilsvrc12/synset_words.txt ../tools/
#download gender models
wget -P ../networks/Gender/ -N https://dl.dropboxusercontent.com/u/38822310/gender_net.caffemodel
#wget -P ../networks/Gender/ -N https://gist.githubusercontent.com/GilLevi/c9e99062283c719c03de/raw/0b42d52c50d9e4f18592adb4fbf6c33132dd4ac7/deploy_gender.prototxt
#download age models
#wget -P ../networks/Age/ -N https://gist.githubusercontent.com/GilLevi/c9e99062283c719c03de/raw/0b42d52c50d9e4f18592adb4fbf6c33132dd4ac7/deploy_age.prototxt
wget -P ../networks/Age/ -N https://dl.dropboxusercontent.com/u/38822310/age_net.caffemodel
#download age and gender mean
wget -P ../mean/age_gender/ -N https://dl.dropboxusercontent.com/u/38822310/mean.binaryproto
python3 convert_mean.py
# get sample image
wget -P ../images/ -N https://github.com/BVLC/caffe/raw/master/examples/images/cat.jpg
cd