import os
import numpy as np
import caffe

proto_data = open('agegender_mean.binaryproto', "rb").read()
a = caffe.io.caffe_pb2.BlobProto.FromString(proto_data)
mean  = caffe.io.blobproto_to_array(a)[0]
np.save('agegender_mean.npy', mean)

proto_data = open('imagenet_mean.binaryproto', "rb").read()
a = caffe.io.caffe_pb2.BlobProto.FromString(proto_data)
mean  = caffe.io.blobproto_to_array(a)[0]
np.save('imagenet_mean.npy', mean)
