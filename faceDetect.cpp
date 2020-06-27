
#include <stdlib.h>
#include <stdexcept>
/* uncomment for applications that use vectors */
/*#include <vector>*/

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
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
#include <thread>
using std::cout;
using std::endl;
using std::vector;
using std::string;
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
            "\tUsing OpenCV version "
         << CV_VERSION << "\n"
         << endl;
}

vector<Rect> detectAndDraw(Mat &img, CascadeClassifier &cascade,
                           CascadeClassifier &nestedCascade,
                           double scale, bool tryflip);

Rect getLargestRect(vector<Rect> images);

static void readCsv(const string &filename, vector<Mat> &images,
                     vector<int> &labels, char separator);

static void loadModel();

static void saveModel();

static void updateModel(Mat image, int label);

static int predictFace(Mat image);

static double predictConfidence(Mat image, int predictedLabel);

static Mat prepareImage(Mat image);

static void test(string trainCsv, string testCsv);

Ptr<FaceRecognizer> model;
string cascadeName;
string nestedCascadeName;
bool tryflip;
CascadeClassifier cascade, nestedCascade;
double scale;
string modelFile = "faces.yml";
string database = "recog";
string table = "test";
string database_user = "root";
string database_password = "password";

int photo_delay = 5;
int photo_amount = 3;
int photo_amount_counter = 0;
double confidence_limit = 30;
bool silent = false;

sql::Driver *driver;
sql::Connection *con;
sql::Statement *stmt;
sql::ResultSet *res;
int main(int argc, const char **argv)
{
    try {
        

        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", database_user, database_password);
        /* Connect to the MySQL database */
        //con->setSchema("recog");

        stmt = con->createStatement();
        stmt->execute("USE " + database);
        stmt->execute("DELETE FROM " + table + " WHERE id=0");
        stmt->execute("INSERT INTO " + table + "(id, status) VALUES (0, 42)");
        res = stmt->executeQuery("SELECT status FROM " + table + " WHERE id=0");
        while (res->next()) {
            cout << res->getString("status") << endl;
        }
        delete res;
        stmt->execute("DELETE FROM " + table + " WHERE id=0");
        

    } catch (sql::SQLException &e) {
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }

    
    Mat frame, image;
    string inputName;

    cv::CommandLineParser parser(argc, argv,
                                 "{help h||}"
                                 "{cascade|data/haarcascades/haarcascade_frontalface_alt.xml|}"
                                 "{nested-cascade|data/haarcascades/haarcascade_eye_tree_eyeglasses.xml|}"
                                 "{scale|1|}{try-flip||}{@filename||}"
                                 "{test||}"
                                 "{scan||}"
                                 "{silent||}"
                                 "{train-csv|train.csv|}"
                                 "{test-csv|test.csv|}");
    if (parser.has("help"))
    {
        help();
        return 0;
    }

    // No image shown
    if (parser.has("silent"))
    {
        silent = true;
    }

    model = LBPHFaceRecognizer::create(1, 4, 8, 8); // the second number has great impact on performance
    loadModel();


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
    //if (!nestedCascade.load(samples::findFileOrKeep(nestedCascadeName)))
    //    std::cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
    if (!cascade.load(samples::findFile(cascadeName)))
    {
        std::cerr << "ERROR: Could not load classifier cascade" << endl;
        help();
        return -1;
    }

    if (parser.has("test"))
    {
        test(parser.get<string>("train-csv"), parser.get<string>("test-csv"));
        return 1;
    }

    int camera = inputName.empty() ? 0 : inputName[0] - '0';
    VideoCapture capture(camera);
    if (inputName.empty() || (isdigit(inputName[0]) && inputName.size() == 1))
    {
        
        if (!capture.isOpened())
        {
            cout << "Capture from camera #" << camera << " didn't work" << endl;
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
    int teach = 0;
    int person_id = 0;
    if (parser.has("scan") && capture.isOpened())
    {
        cout << "Scanning started" << endl;
        for (;;)
        {
            if(!con->isValid()){
                cout << "Database connection invalid, attempting to reconnect..." << endl;
                try{
                    con->reconnect();
                } catch (sql::SQLException &e) {
                    cout << "# ERR: SQLException in " << __FILE__;
                    cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
                    cout << "# ERR: " << e.what();
                    cout << " (MySQL error code: " << e.getErrorCode();
                    cout << ", SQLState: " << e.getSQLState() << " )" << endl;
                }
            }
            capture >> frame;
            if (frame.empty())
            {
                break;
            }
            // Check whether the person in frame should be taught to the model
            res = stmt->executeQuery("SELECT person_id FROM " + table + " WHERE id=1");
            while (res->next()) {
                int temp_counter = photo_delay;
                while(temp_counter > 0){
                    cout << temp_counter << endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    temp_counter = temp_counter - 1;
                }
                teach = 1;
                person_id = res->getInt("person_id");
                stmt->execute("DELETE FROM " + table + " WHERE id=1");
                photo_amount_counter = 0;
            }
            delete res;

            Mat frame1 = frame.clone();
            Mat largestFace;
            vector<Rect> images = detectAndDraw(frame1, cascade, nestedCascade, scale, tryflip);
            Rect largestRect = getLargestRect(images);
            frame1.copyTo(largestFace);
            if (largestRect.width <= 0)
            {
                stmt->execute("DELETE FROM " + table + " WHERE id=0");
                stmt->execute("INSERT INTO " + table + "(id, status) VALUES (0, 0)");
                cout << "." << std::flush;
                continue;
            }
            //string data =  format("x = %d / Y = %d / width = %d / Height = %d", cvRound(largestRect.x),cvRound(largestRect.y), largestRect.width-1, largestRect.height-1);
            //cout << data << endl;
            Mat croppedFace(largestFace, Rect(cvRound(largestRect.x), cvRound(largestRect.y), largestRect.width - 1, largestRect.height - 1));

            char c = (char)waitKey(10);
            if (c == 27 || c == 'q' || c == 'Q')
            {
                break;
            }
            
            Mat processedImage = prepareImage(croppedFace);
            if(teach)
            {   
                if(photo_amount_counter < photo_amount)
                {
                    photo_amount_counter += 1;
                    cout << "Teach face with id " << std::to_string(person_id) << ". Photo " << photo_amount_counter << " out of " << photo_amount << endl;
                    updateModel(processedImage, person_id);
                } 
                else 
                {
                    saveModel();
                    teach = 0;
                    person_id = 0;
                    photo_amount_counter == 0;
                }
            }
            else 
            {
                // Detect who the person is
                int id = -1;
                double confidence = 0.0;
                model->predict(processedImage, id, confidence);
                string resultMessage = format("Predicted class = %02d / Confidence = %.0f ", id, confidence);
                // Replace this part with writing to database when we get to there
                if (confidence > confidence_limit)
                {
                    stmt->execute("DELETE FROM " + table + " WHERE id=2");
                    stmt->execute("INSERT INTO " + table + "(id, status) VALUES (2, 0)");
                    cout << "Not recognized, status: 0" << endl;
                }
                else {
                    stmt->execute("DELETE FROM " + table + " WHERE id=2");
                    stmt->execute("INSERT INTO " + table + "(id, person_id, confidence, status) VALUES (2, " + std::to_string(id) + ", " + std::to_string(confidence) + ", 1)");
                    cout << "Recognized, status: 1, person:" + std::to_string(id) + ", confidence: " + std::to_string(confidence) << endl;
                }
            }
        }
    }

    if (capture.isOpened())
    {
        cout << "Video capturing has been started ..." << endl;
        for (;;)
        {
            capture >> frame;
            if (frame.empty())
                break;
            Mat frame1 = frame.clone();
            Mat largestFace;
            vector<Rect> images = detectAndDraw(frame1, cascade, nestedCascade, scale, tryflip);
            Rect largestRect = getLargestRect(images);
            frame1.copyTo(largestFace);
            if (largestRect.width <= 0)
            {
                continue;
            }
            //string data =  format("x = %d / Y = %d / width = %d / Height = %d", cvRound(largestRect.x),cvRound(largestRect.y), largestRect.width-1, largestRect.height-1);
            //cout << data << endl;
            Mat croppedFace(largestFace, Rect(cvRound(largestRect.x), cvRound(largestRect.y), largestRect.width - 1, largestRect.height - 1));

            char c = (char)waitKey(10);
            if (c == 27 || c == 'q' || c == 'Q')
            {
                delete res;
                delete stmt;
                delete con;
                break;
            }
            else if (47 < c && c < 58) // if number is pressed
            {

                int id = c - 48; // convert ascii to numbers
                Mat processedImage = prepareImage(croppedFace);
                if (!silent)
                {
                    imshow("The face that would we teached", croppedFace);
                    imshow("Processed version", processedImage);
                }
                cout << "Teach this with id " << std::to_string(id) << "?" << endl;
                // Wait for the user to confirm the teaching, if space is pressed tech, otherwise discard
                char c2 = (char)waitKey(0);
                if (c2 == 32)
                {
                    cout << "Teach face with id " << std::to_string(id) << endl;
                    updateModel(processedImage, id);
                }
                else
                {
                    cout << "Discarded" << endl;
                }
            }
            else if (c == 100) // letter d
            {
                // Detect who the person is
                Mat processedImage = prepareImage(croppedFace);
                int id = -1;
                double confidence = 0.0;
                model->predict(processedImage, id, confidence);
                string resultMessage = format("Predicted class = %02d / Confidence = %.0f ", id, confidence);
                cout << resultMessage << endl;
            }
            else if (c == 's')
            {
                saveModel();
                cout << "Model saved to file" << endl;
            }
            else if (c == 'l')
            {
                cout << "Trying to load a model file" << endl;
                loadModel();
            }
            else if (c == 'c')
            {
                cout << "Model cleared" << endl;
                model->clear();
            }
        }
    }
    else
    {
        cout << "Detecting face(s) in " << inputName << endl;
        if (!image.empty())
        {
            detectAndDraw(image, cascade, nestedCascade, scale, tryflip);
            waitKey(0);
        }
        else if (!inputName.empty())
        {
            /* assume it is a text file containing the
            list of the image filenames to be processed - one per line */
            FILE *f = fopen(inputName.c_str(), "rt");
            if (f)
            {
                char buf[1000 + 1];
                while (fgets(buf, 1000, f))
                {
                    int len = (int)strlen(buf);
                    while (len > 0 && isspace(buf[len - 1]))
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread(buf, 1);
                    if (!image.empty())
                    {
                        detectAndDraw(image, cascade, nestedCascade, scale, tryflip);
                        char c = (char)waitKey(0);
                        if (c == 27 || c == 'q' || c == 'Q')
                            break;
                    }
                    else
                    {
                        std::cerr << "Aw snap, couldn't read image " << buf << endl;
                    }
                }
                fclose(f);
            }
        }
    }
    return 0;
}

vector<Rect> detectAndDraw(Mat &img, CascadeClassifier &cascade,
                           CascadeClassifier &nestedCascade,
                           double scale, bool tryflip)
{
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
        {
            Scalar(255, 0, 0),
            Scalar(255, 128, 0),
            Scalar(255, 255, 0),
            Scalar(0, 255, 0),
            Scalar(0, 128, 255),
            Scalar(0, 255, 255),
            Scalar(0, 0, 255),
            Scalar(255, 0, 255)};
    Mat gray, smallImg;
    cout << "5.1" << endl;
    cvtColor(img, gray, COLOR_BGR2GRAY); // Kaatuu tässä
    cout << "5.2" << endl;
    double fx = 1 / scale;
    resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
    equalizeHist(smallImg, smallImg);
    t = (double)getTickCount();

    cascade.detectMultiScale(smallImg, faces,
                             1.1, 2, 0
                             //|CASCADE_FIND_BIGGEST_OBJECT
                             //|CASCADE_DO_ROUGH_SEARCH
                             | CASCADE_SCALE_IMAGE,
                             Size(30, 30));
    if (tryflip)
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale(smallImg, faces2,
                                 1.1, 2, 0
                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                 //|CASCADE_DO_ROUGH_SEARCH
                                 | CASCADE_SCALE_IMAGE,
                                 Size(30, 30));
        for (vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); ++r)
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)getTickCount() - t;
    // printf( "detection time = %g ms\n", t*1000/getTickFrequency());

    Mat cleanImg, largestFace;
    img.copyTo(cleanImg);
    Rect largestRect = getLargestRect(faces);

    for (size_t i = 0; i < faces.size(); i++)
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i % 8];

        Mat croppedFace(cleanImg, Rect(cvRound(r.x * scale), cvRound(r.y * scale), r.width * scale - 1, r.height * scale - 1));
        // This is what we want to give to recognition software
        if (faces[i] == largestRect)
        {
            largestFace = cleanImg;
            if (!silent)
            {
                imshow("Largest face", croppedFace);
            }
        }
        //imshow("Face" + std::std::to_string(i), croppedFace);

        rectangle(img, Point(cvRound(r.x * scale), cvRound(r.y * scale)),
                  Point(cvRound((r.x + r.width - 1) * scale), cvRound((r.y + r.height - 1) * scale)),
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
    if (!silent)
    {
        imshow("Camera feed", img);
    }
    return faces;
}

Rect getLargestRect(vector<Rect> rects)
{
    Rect largestRect(0, 0, 0, 0);
    for (size_t i = 0; i < rects.size(); ++i)
    {
        Rect rect = rects[i];
        if (rect.width > largestRect.width)
        {
            largestRect = rect;
        }
    }
    return largestRect;
}

Mat prepareImage(Mat image)
{
    Mat processedImg;

    image.copyTo(processedImg);
    cvtColor(image, processedImg, COLOR_BGR2GRAY);
    equalizeHist(processedImg, processedImg); // Does this work as intended? Should the output image be different?
    return processedImg;
}

static void readCsv(const string &filename, vector<Mat> &images, vector<int> &labels, char separator = ';')
{
    std::ifstream file(filename.c_str(), std::ifstream::in);
    if (!file)
    {
        string errorMessage = "No valid input file was given, please check the given filename.";
        CV_Error(Error::StsBadArg, errorMessage);
    }
    string line, path, classlabel;
    while (getline(file, line))
    {
        std::stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if (!path.empty() && !classlabel.empty())
        {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

static void loadModel()
{
    // TODO: Check if exists already, if so, load it and do not create new
    std::ifstream file;
    file.open(modelFile); //Load model file
    if (file)
    {
        file.close();
        model->read(modelFile);
        cout << "Model loaded" << endl;
    }
    else
    {
        cout << "No model file found" << endl;
    }
}

static void saveModel()
{
    model->save(modelFile);
}

static void updateModel(Mat image, int label)
{
    vector<Mat> newImage;
    vector<int> newLabel;
    newImage.push_back(image);
    newLabel.push_back(label);
    model->update(newImage, newLabel);
}

static int predictFace(Mat image)
{
    return model->predict(image);
}

static double predictConfidence(Mat image, int predictedLabel)
{
    double confidence;
    model->predict(image, predictedLabel, confidence);
    return confidence;
}

// For teting purposes, I bet you didn't guess that
static void test(string trainCsv, string testCsv)
{
    // Get the path to your CSV.
    string fnCsv = trainCsv;
    string fnTestCsv = testCsv;
    // These vectors hold the images and corresponding labels.
    vector<Mat> images;
    vector<int> labels;
    vector<Mat> testImages;
    vector<int> testLabels;
    // Read in the data. This can fail if no valid
    // input filename is given.
    cout << "1" << endl;
    try
    {
        readCsv(fnCsv, images, labels);
    }
    catch (const cv::Exception &e)
    {
        std::cerr << "Error opening file \"" << fnCsv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    // Read in the test data
    cout << "2" << endl;
    try
    {
        readCsv(fnTestCsv, testImages, testLabels);
    }
    catch (const cv::Exception &e)
    {
        std::cerr << "Error opening file \"" << fnTestCsv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    cout << "3" << endl;
    // Quit if there are not enough images for this demo.
    if (images.size() < 1)
    {
        string errorMessage = "This demo needs at least 1 image to work. Please add more images to your data set!";
        CV_Error(Error::StsError, errorMessage);
    }
    // Get the height from the first image. We'll need this
    // later in code to reshape the images to their original
    // size:
    int height = images[0].rows;
    cout << "4" << endl;
    for (int i = 0; i < images.size(); i++)
    {
        Mat frame1 = images[i];
        cout << "5" << endl;
        Mat largestFace;
        vector<Rect> images = detectAndDraw(frame1, cascade, nestedCascade, scale, tryflip);
        cout << "6" << endl;
        Rect largestRect = getLargestRect(images);
        cout << "7" << endl;
        frame1.copyTo(largestFace);
        cout << "8" << endl;
        if (largestRect.width <= 0)
        {
            cout << "No face found" << endl;
            continue;
        }
        //string data =  format("x = %d / Y = %d / width = %d / Height = %d", cvRound(largestRect.x),cvRound(largestRect.y), largestRect.width-1, largestRect.height-1);
        //cout << data << endl;
        Mat croppedFace(largestFace, Rect(cvRound(largestRect.x), cvRound(largestRect.y), largestRect.width - 1, largestRect.height - 1));
        cout << "9" << endl;
        Mat processedImage = prepareImage(croppedFace);
        updateModel(processedImage, labels[i]);
    }
    int correct = 0;
    int wrong = 0;
    double elapsed = 0;
    for (int i = 0; i < testImages.size(); i++)
    {
        int predictedLabel = -1;
        double confidence = 0.0;
        auto start = std::chrono::high_resolution_clock::now();
        predictedLabel = predictFace(testImages[i]);
        confidence = predictConfidence(testImages[i], predictedLabel);
        auto finish = std::chrono::high_resolution_clock::now();
        double duration = (std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count()) / 1000000.0;

        string resultMessage = format("Predicted class = %02d / Actual class = %02d / Confidence = %.0f / Time =  %.4fs", predictedLabel, testLabels[i], confidence, duration);
        if (predictedLabel == testLabels[i])
        {
            correct++;
        }
        else
        {
            wrong++;
        }
        elapsed += duration;
        cout << resultMessage << endl;
    }
    double accuracy = 1.0 * correct / (correct + wrong) * 100;
    double averageFps = testImages.size() / elapsed;
    cout << format("Correct: %d / Wrong: %d / Accuracy: %.2f%% / FPS: %.2f", correct, wrong, accuracy, averageFps) << endl;
}