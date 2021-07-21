#!/usr/bin/env python

import sys
import os.path

# Loop through folders in FERET/colorferet/dvd1/data/images/, and add images that are used for training to one csv and testing images to another. 
# Also check the name of the images, to filter only desired images (eg only frontal images) 
# also things

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("usage: create_csv <base_path> <images.txt>")     
        sys.exit(1)

    BASE_PATH=sys.argv[1]
    IMAGES_NAME=sys.argv[2]
    SEPARATOR=";"

    images = open(IMAGES_NAME)
    for line in images:
        temp = line.split(" ")
        label = temp[0]
        if(len(temp) > 2):
            filename = temp[1]
        else:
            filename = temp[1][:-1]
        
        subject_path = os.path.join(BASE_PATH, label)
        abs_path = "%s/%s" % (subject_path, filename)
        print("%s%s%d" % (abs_path, SEPARATOR, int(label)))

    images.close()
    #label = 0
    #for dirname, dirnames, filenames in os.walk(BASE_PATH):
    #    for subdirname in dirnames:
    #        subject_path = os.path.join(dirname, subdirname)
    #        for filename in os.listdir(subject_path):
    #            abs_path = "%s/%s" % (subject_path, filename)
    #            print "%s%s%d" % (abs_path, SEPARATOR, label)
    #        label = label + 1
