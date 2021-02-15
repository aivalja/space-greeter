#/bin/bash

./detect.o 0 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1 --test --silent --single --radius=8 --neighbours=6
