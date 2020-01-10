# space-greeter
To compile and run: 
``` bash
g++ recognize.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./main.o train.csv test.csv test.csv ./output_dir/

g++ faceDetect.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./detect.o --camera=2 --eyes_cascade=haarcascades/haarcascade_eye.xml --face_cascade=haarcascades/haarcascade_frontalface_default.xml 

```

