# space-greeter
To compile and run: 
``` bash

g++ faceDetect.cpp `pkg-config opencv4 --cflags --libs` -g -o detect.o
./detect.o 2 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1


g++ recognize.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./main.o train.csv test.csv test.csv ./output_dir/

```
recognize.cpp is now obsolete, all functionality has been transferred to faceDetect.cpp

Check if squares are seen in teaching images