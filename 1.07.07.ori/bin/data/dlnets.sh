wget http://dl.caffe.berkeleyvision.org/bvlc_alexnet.caffemodel -O alexnet.caffemodel
wget https://dl.dropboxusercontent.com/u/38822310/age_net.caffemodel -O age.caffemodel
wget https://dl.dropboxusercontent.com/u/38822310/gender_net.caffemodel -O gender.caffemodel
wget http://dl.caffe.berkeleyvision.org/bvlc_googlenet.caffemodel -O googlenet.caffemodel
wget https://github.com/DeepScale/SqueezeNet/blob/master/SqueezeNet_v1.0/squeezenet_v1.0.caffemodel -O squeezenet.caffemodel

wget http://dl.caffe.berkeleyvision.org/caffe_ilsvrc12.tar.gz
tar xf caffe_ilsvrc12.tar.gz imagenet_mean.binaryproto && rm -f caffe_ilsvrc12.tar.gz
wget https://dl.dropboxusercontent.com/u/38822310/mean.binaryproto -O agegender_mean.binaryproto
python3 generate_means.py
rm imagenet_mean.binaryproto agegender_mean.binaryproto
