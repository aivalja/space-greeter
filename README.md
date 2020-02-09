# space-greeter
To compile and run: 
``` bash

g++ faceDetect.cpp `pkg-config opencv4 --cflags --libs` -g -o detect.o
/detect.o 2 --cascade=haarcascades/haarcascade_frontalface_alt.xml --scale=1


g++ recognize.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./main.o train.csv test.csv test.csv ./output_dir/

```
recognize.cpp is now obsolete, all functionality has been transferred to faceDetect.cpp

ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄHHHADKFLJKDSJ
The images should be mat:s, not Rect:s. So we have to redo multiple functions