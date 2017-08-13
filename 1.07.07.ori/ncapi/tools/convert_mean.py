import caffe
import numpy as np
import sys
import os

blob = caffe.proto.caffe_pb2.BlobProto()
data = open( '../mean/age_gender/mean.binaryproto', 'rb' ).read()
blob.ParseFromString(data)
arr = np.array( caffe.io.blobproto_to_array(blob) )
out = arr[0]
np.save( '../mean/age_gender/age_gender_mean.npy', out )
blob = caffe.proto.caffe_pb2.BlobProto()
data = open( '/opt/movidius/caffe/data/ilsvrc12/imagenet_mean.binaryproto', 'rb' ).read()
blob.ParseFromString(data)
arr = np.array( caffe.io.blobproto_to_array(blob) )
out = arr[0]
os.system('mkdir ../mean/ilsvrc12')
np.save( '../mean/ilsvrc12/ilsvrc_2012_mean.npy', out )
