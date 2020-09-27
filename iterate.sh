#/bin/bash

#./detect.o 0 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1 --test --silent --radius=1 --neighbours=3

for radius in 1 2 3 4 5 6 7 8 9 10 14 20 30 
do
    for neighbours in 1 2 3 4 5 6 7 8 9 10
    do
        # $ "run --radius=$radius --neighbours=$neighbours"
        ./detect.o 0 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1 --test --silent --radius=$radius --neighbours=$neighbours
    done
done
