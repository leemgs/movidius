#!/bin/sh

NCS_TOOLKIT_ROOT='../../bin'
echo $NCS_TOOLKIT_ROOT
python3 $NCS_TOOLKIT_ROOT/mvNCCompile.pyc ../networks/SqueezeNet/NetworkConfig.prototxt -w ../networks/SqueezeNet/squeezenet_v1.0.caffemodel -o ../networks/SqueezeNet/graph
python3 $NCS_TOOLKIT_ROOT/mvNCCompile.pyc ../networks/GoogLeNet/NetworkConfig.prototxt -w ../networks/GoogLeNet/bvlc_googlenet.caffemodel -o ../networks/GoogLeNet/graph
python3 $NCS_TOOLKIT_ROOT/mvNCCompile.pyc ../networks/Gender/NetworkConfig.prototxt -w ../networks/Gender/gender_net.caffemodel -o ../networks/Gender/graph
python3 $NCS_TOOLKIT_ROOT/mvNCCompile.pyc ../networks/Age/deploy_age.prototxt -w ../networks/Age/age_net.caffemodel -o ../networks/Age/graph
python3 $NCS_TOOLKIT_ROOT/mvNCCompile.pyc ../networks/AlexNet/NetworkConfig.prototxt -w ../networks/AlexNet/bvlc_alexnet.caffemodel -o ../networks/AlexNet/graph
