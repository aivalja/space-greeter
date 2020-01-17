# space-greeter
To compile and run: 
``` bash
g++ recognize.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./main.o train.csv test.csv test.csv ./output_dir/

g++ faceDetect.cpp `pkg-config opencv4 --cflags --libs` -g -o detect.o
/detect.o 2 --cascade=lbpcascades/lbpcascade_frontalface_improved.xml --scale=1


python3 main.py
```
Keep detect.o running while running main.py

for detect.o, scale of 1.5 seems to work well, lbpcascade improved seemed to work better than haarcascade

TODO:
Create a script for compiling everything correctly
Face is detected as a square, it might decrease the classification accuracy 
