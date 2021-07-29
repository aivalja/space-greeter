# space-greeter

This is a project for creating a raspberry pi powered face recognition software for "greeting" by name people that the camera sees. For backend, OpenCV should be installed. If you want to run tests, FERET database needs to be downloaded and extracted.

To compile and run: 
``` bash

g++ faceDetect.cpp -g -I /usr/local/include/opencv4/ -lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_highgui -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_quality -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_videostab -lopencv_videoio -lopencv_viz -lopencv_xfeatures2d -lopencv_shape -lopencv_ml -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core  -o detect.o
```
#To run the program in detection mode
``` bash
./detect.o 0 --cascade=lbpcascades/lbpcascade_frontalface.xml --scan --silent
```
To run tests:
Use data sorter to put training and test sets from FERET folder to train.csv and test.csv respectively

Using data-sorter.py:
``` bash
python data-sorter.py /path/to/feret_folder/FERET/colorferet/dvd1/data/images/ /path/to/feret_folder/FERET/colorferet/dvd1/doc/partitions/fb.txt > test.csv
```
Next you can run tests either manually:
``` bash
./detect.o 0 --cascade=$cascade --scale=$scale --test --silent --radius=$radius --neighbours=$neighbours --dataset=$dataset --$single
```
Or alternatively edit variables to be tested in iterate.sh and run it.



**User interface setup**
1. Install PHP and XAMPP or similar program to run Apache and MySQL modules. XAMPP Control panel v3.2.4, PHP 7.3.14 and Apache 2.4.38 has been proven to work
2. Move to all the files in the UI folder to XAMPP's htdocs folder.
3. The PHP files have hard coded variables in the beginning, such as database's password and username. Change them if needed.
4. Run XAMPP's Apache and MySQL modules and the user interface should be seen in localhost/the folder name that holds the UI files
