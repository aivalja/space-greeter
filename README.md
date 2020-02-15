# space-greeter
To compile and run: 
``` bash

g++ faceDetect.cpp `pkg-config opencv4 --cflags --libs` -g -o detect.o
./detect.o 2 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1


g++ recognize.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./main.o train.csv test.csv test.csv ./output_dir/

```

While running:
* press q to quit
* Press number key to teach that face to with matching id, confirm with space or discard with any key
* Press d to predict the id of face
Use only faceDetect.cpp, recognize is obsolete

Todo:
Check if squares are seen in teaching images
If empty model, crashes