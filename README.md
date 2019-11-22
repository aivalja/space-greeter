# space-greeter
To compile and run: 
``` bash
g++ recognize.cpp `pkg-config opencv4 --cflags --libs` -g -o main.o
./main.o train.csv test.csv ./output_dir/
```

