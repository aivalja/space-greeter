#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
using namespace std;
using namespace cv;
using namespace cv::face;

static void help()
{
    cout << "\nThis program demonstrates the use of cv::CascadeClassifier class to detect objects (Face + eyes). You can use Haar or LBP features.\n"
            "This classifier can recognize many kinds of rigid objects, once the appropriate classifier is trained.\n"
            "It's most known use is for faces.\n"
            "Usage:\n"
            "./facedetect [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
               "   [--nested-cascade[=nested_cascade_path this an optional secondary classifier such as eyes]]\n"
               "   [--scale=<image scale greater or equal to 1, try 1.3 for example>]\n"
               "   [--try-flip]\n"
               "   [filename|camera_index]\n\n"
            "see facedetect.cmd for one call:\n"
            "./facedetect --cascade=\"data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"data/haarcascades/haarcascade_eye_tree_eyeglasses.xml\" --scale=1.3\n\n"
            "During execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

 
vector<Rect> detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip );

Rect getLargestImage( vector<Rect> images);

static void read_csv( const string& filename, vector<Mat>& images,
                      vector<int>& labels, char separator);

static void load_model();

static void save_model();

static void update_model(Mat image, int label);

static int predict_face(Mat image);

static double precit_confidence(Mat image, int predictedLabel);

static void test();

Ptr<FaceRecognizer> model;
string cascadeName;
string nestedCascadeName;
string model_file = "faces.yml";

int main( int argc, const char** argv ){
    VideoCapture capture;
    Mat frame, image;
    string inputName;
    bool tryflip;
    CascadeClassifier cascade, nestedCascade;
    double scale;
    cv::CommandLineParser parser(argc, argv,
        "{help h||}"
        "{cascade|data/haarcascades/haarcascade_frontalface_alt.xml|}"
        "{nested-cascade|data/haarcascades/haarcascade_eye_tree_eyeglasses.xml|}"
        "{scale|1|}{try-flip||}{@filename||}"
    );
    if (parser.has("help"))
    {
        help();
        return 0;
    }

    //Testing purposes
    //test();

    load_model();

    cascadeName = parser.get<string>("cascade");
    nestedCascadeName = parser.get<string>("nested-cascade");
    scale = parser.get<double>("scale");
    if (scale < 1)
        scale = 1;
    tryflip = parser.has("try-flip");
    inputName = parser.get<string>("@filename");
    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }
    if (!nestedCascade.load(samples::findFileOrKeep(nestedCascadeName)))
        cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
    if (!cascade.load(samples::findFile(cascadeName)))
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        help();
        return -1;
    }
    if( inputName.empty() || (isdigit(inputName[0]) && inputName.size() == 1) )
    {
        int camera = inputName.empty() ? 0 : inputName[0] - '0';
        if(!capture.open(camera))
        {
            cout << "Capture from camera #" <<  camera << " didn't work" << endl;
            return 1;
        }
    }
    else if (!inputName.empty())
    {
        image = imread(samples::findFileOrKeep(inputName), IMREAD_COLOR);
        if (image.empty())
        {
            if (!capture.open(samples::findFileOrKeep(inputName)))
            {
                cout << "Could not read " << inputName << endl;
                return 1;
            }
        }
    }

    if( capture.isOpened() )
    {
        cout << "Video capturing has been started ..." << endl;
        for(;;)
        {
            capture >> frame;
            if( frame.empty() )
                break;
            Mat frame1 = frame.clone();
            detectAndDraw( frame1, cascade, nestedCascade, scale, tryflip );
            char c = (char)waitKey(10);
            if( c == 27 || c == 'q' || c == 'Q' )
            {
                break;
            }
            else if( 47 < c && c < 58) // if number is pressed
            {
                vector<Rect> images = detectAndDraw( frame1, cascade, nestedCascade, scale, tryflip );
                Rect face = getLargestImage(images);
                //update_model()
            }
        }
    }
    else
    {
        cout << "Detecting face(s) in " << inputName << endl;
        if( !image.empty() )
        {
            detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
            waitKey(0);
        }
        else if( !inputName.empty() )
        {
            /* assume it is a text file containing the
            list of the image filenames to be processed - one per line */
            FILE* f = fopen( inputName.c_str(), "rt" );
            if( f )
            {
                char buf[1000+1];
                while( fgets( buf, 1000, f ) )
                {
                    int len = (int)strlen(buf);
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread( buf, 1 );
                    if( !image.empty() )
                    {
                        detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
                        char c = (char)waitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                    }
                    else
                    {
                        cerr << "Aw snap, couldn't read image " << buf << endl;
                    }
                }
                fclose(f);
            }
        }
    }
    return 0;
}

vector<Rect> detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip ) {
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;
    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    equalizeHist( smallImg, smallImg );
    t = (double)getTickCount();
    
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 2, 0
                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                 //|CASCADE_DO_ROUGH_SEARCH
                                 |CASCADE_SCALE_IMAGE,
                                 Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); ++r )
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)getTickCount() - t;
    // printf( "detection time = %g ms\n", t*1000/getTickFrequency());

    Mat clean_img;
    img.copyTo(clean_img);
    Rect largestFace = getLargestImage(faces);
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];

        Mat cropped_face(clean_img, Rect(cvRound(r.x*scale),cvRound(r.y*scale),r.width*scale-1, r.height*scale-1));
        // This is what we want to give to recognition software
        if(faces[i] == largestFace){
            imshow("Largest face", cropped_face);
        }
        imshow("Face" + std::to_string(i), cropped_face);
        

        rectangle( img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
                   Point(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                   color, 3, 8, 0);
        
        // This part can be used to detect objects (for example eyes) inside the objects
        /*
        if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            |CASCADE_SCALE_IMAGE,
            Size(30, 30) );
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        */
    }
    imshow( "result", img );
    return faces;
}

Rect getLargestImage( vector<Rect> images){
    Rect largestRect(0,0,0,0);
    for(size_t i = 0; i < images.size(); ++i){
        Rect rect = images[i];
        if(rect.width > largestRect.width){
            largestRect = rect;
        }
    }
    return largestRect;
}

Mat prepareImage(Rect image){
    Mat processed_img;
    image.copyTo(processed_img);
    return processed_img;
}

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(Error::StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

static void load_model() {
    // TODO: Check if exists already, if so, load it and do not create new
    model = LBPHFaceRecognizer::create(1, 4, 8, 8); // the second number has great impact on performance
}

static void save_model() {
    model->save(model_file);
}

static void update_model(Mat image, int label){
    vector<Mat> new_image;
    vector<int> new_label;
    new_image.push_back(image);
    new_label.push_back(label);
    model->update(new_image,new_label);
}

static int predict_face(Mat image){
    return model->predict(image);
}

static double predict_confidence(Mat image, int predictedLabel){
    double confidence;
    model->predict(image, predictedLabel, confidence);
    return confidence;
}

// For teting purposes, I bet you didn't guess that
static void test(){
    // Get the path to your CSV.
    string fn_csv = "train.csv";
    string fn_test_csv = "test.csv";
    // These vectors hold the images and corresponding labels.
    vector<Mat> images;
    vector<int> labels;
    vector<Mat> test_images;
    vector<int> test_labels;
    // Read in the data. This can fail if no valid
    // input filename is given.
    try {
        read_csv(fn_csv, images, labels);
    } catch (const cv::Exception& e) {
        cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    // Read in the test data
    try {
        read_csv(fn_test_csv, test_images, test_labels);
    } catch (const cv::Exception& e) {
        cerr << "Error opening file \"" << fn_test_csv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    // Quit if there are not enough images for this demo.
    if(images.size() < 1) {
        string error_message = "This demo needs at least 1 image to work. Please add more images to your data set!";
        CV_Error(Error::StsError, error_message);
    }
    // Get the height from the first image. We'll need this
    // later in code to reshape the images to their original
    // size:
    int height = images[0].rows;

    load_model();
    for (int i = 0; i < images.size(); i++){
        update_model(images[i], labels[i]);
    }
    int correct = 0;
    int wrong = 0;
    double elapsed = 0;
    for (int i = 0; i < test_images.size(); i++)
    {
        int predictedLabel = -1;
        double confidence = 0.0;
        auto start = std::chrono::high_resolution_clock::now();
        predictedLabel = predict_face(test_images[i]);
        confidence = predict_confidence(test_images[i], predictedLabel);
        auto finish = std::chrono::high_resolution_clock::now();
        double duration = (std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count())/1000000.0;
        
        string result_message = format("Predicted class = %02d / Actual class = %02d / Confidence = %.0f / Time =  %.4fs", predictedLabel, test_labels[i], confidence, duration);
        if(predictedLabel == test_labels[i]){
            correct++;
        } else {
            wrong++;
        }
        elapsed += duration;
        cout << result_message << endl;
    }
    double accuracy = 1.0*correct/(correct + wrong)*100;
    double average_fps = test_images.size()/elapsed;
    cout << format("Correct: %d / Wrong: %d / Accuracy: %.2f%% / FPS: %.2f", correct, wrong, accuracy, average_fps) << endl;
    
}