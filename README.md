# space-greeter
To compile and run: 
``` bash

g++ faceDetect.cpp -g -I /usr/local/include/opencv4/ -lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_highgui -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_quality -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_videostab -lopencv_videoio -lopencv_viz -lopencv_xfeatures2d -lopencv_shape -lopencv_ml -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core  -o detect.o

./detect.o 2 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1.

./detect.o 2 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1 --test --train-csv=train.csv --test-csv=test.csv

```

While running:
* press q to quit
* Press number key to teach that face to with matching id, confirm with space or discard with any key
* Press d to predict the id of face
Use only faceDetect.cpp, recognize is obsolete

If cannot stream image over ssh, sudo su and
xauth merge /home/aiva/.Xauthority

Todo:
Check if squares are seen in teaching images
If empty model, crashes

mysql connector legacy version 1.1

