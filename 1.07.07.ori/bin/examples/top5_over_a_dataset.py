#! /usr/bin/env python3
"""
 An example userscript for calculating the top-5 accuracy of a network over a given dataset.

 Arguments:
   [0] Directory containing images & labels.txt (relative to Fathom root, labels sorted in numerical file order)
   [1] Network Description (relative to Fathom root)
   [2] Network Weights (relative to Fathom root)

   Example Call:
   python3 top5_over_a_dataset.py examples/data/test/ data/lenet8.prototxt data/Lenet8.caffemodel


"""

import os
import sys
import numpy as np

def check_match(output, expected):
    """
    Check our top1, top5 match for this image

    :param output: an output vector describing classification percentages
    :param expected: the expected label
    :return: correct classification, in the top 5 classifications
    """
    data = output.flatten()
    expected = int(expected)

    sorted_data = np.argsort(data)
    sorted_data = sorted_data[::-1]

    top1 = (expected == sorted_data[0])
    top5 =(expected in sorted_data[0:5])

    return top1, top5


def main():

    # Parse Argument or provide default test case
    if len(sys.argv) < 2:
        print("Using Example Data Directory. For Customization see file header")
        data_directory = "examples/data/test/"
        print("Using Default Data Directory")

    else:
        data_directory =  sys.argv[1]
        # TODO: Assert if not found

    if len(sys.argv) < 3:
        # print("Using Example Description Directory. For Customization see file header")
        network_desc = "data/lenet8.prototxt"
        print("Using Default Description")
    else:
        network_desc = sys.argv[2]
        # TODO: Assert if not found

    if len(sys.argv) < 4:
        print("Using Example Weight Directory. For Customization see file header")
        network_weights = "data/Lenet8.caffemodel"
        print("Using Default Weights")
    else:
        network_weights = sys.argv[3]
        # TODO: Assert if not found


    import re

    # Open labels.txt - a file with classification indexes seperated by newlines

    print(data_directory+"test.txt")
    with open(data_directory+"test.txt") as f:
        labels = re.split('\s', f.read())
        # labels = f.read().split("\n ")
    labels = labels[:-1]
    labels = [re.sub(r'(.*?)\/', '', a,count=2) for a in labels]    # Get rid of first 2 directories, not relevant

    files = []
    indexes = []
    for idx, a  in enumerate(labels):
        if idx % 2 == 1:
            indexes.extend(labels[idx])
        else:
            files.append((labels[idx]))


    # Move to Fathom's running directory


    data_amount = 400 #len(files)

    # Remove old NPY files
    os.system("rm *.npy")

    # Run over our image set with Fathom & produce our classification results

    data_directory = data_directory

    print(os.getcwd())


    for i in range(data_amount):
        print("data_directory + files[i] : ", data_directory + files[i])

        os.system("python3 ./mvNCCheck.pyc -s 12 "+ network_desc + " -i " + data_directory + files[i] + " -id " + str(indexes[i]))
        print    ("python3 ./mvNCCheck.pyc -s 12 "+ network_desc + " -i " + data_directory + files[i] + " -id " + str(indexes[i]))
        os.rename("output_expected.npy", "output"+str(i) + "_expected.npy")

    # Check the match-up of Fathom's results to the true labels
    top1_store = []
    top5_store = []
    for i in range(data_amount):
        res = np.load("output"+str(i)+"_expected.npy")
        top1, top5 = check_match(res, indexes[i])

        top1_store.append(top1)
        top5_store.append(top5)

    print("Top-1 Accuracy: " + str( sum(top1_store)/len(top1_store) * 100 ) + "%")
    print("Top-5 Accuracy: " + str( sum(top5_store)/len(top5_store) * 100) + "%")


if __name__ == "__main__":
    main()
