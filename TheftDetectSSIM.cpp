#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <windows.h>
/**********************************************************************************************************************************
code defines a function named "difference" that takes two input parameters of type cv::Mat (which represents an image frame). 
The function calculates the difference between the two input frames using the structural similarity index (SSIM) algorithm. 
Inside the function, the code first creates four cv::Mat objects: grayA, medianA, grayB, and medianB. 
These objects will be used to store intermediate results during the calculation. 
The code then converts the first input frame (frame1) from the BGR color space to grayscale using the cv::cvtColor function, and 
stores the result in the grayA object. 
After that, it applies a median blur filter with a kernel size of 5x5 to the grayA image using the cv::medianBlur function, and 
stores the blurred image in the medianA object. 
Next, the code performs the same operations on the second input frame (frame2), converting it to grayscale and applying a 
median blur filter, and stores the results in the grayB and medianB objects, respectively. 
After that, the code creates a cv::Mat object named "diff" to store the difference between the medianA and medianB images. 
It then calls the cv::compareStructuralSimilarity function, passing in the medianA and medianB images, as well as the diff object. 
This function calculates the structural similarity index between the two images and stores the result in the "score" variable. 
The code then multiplies the diff image by 255 to scale it to the range of 0-255, and 
converts it to an 8-bit unsigned integer (CV_8U) using the cv::convertTo function. 
Finally, the function returns the calculated score.
**********************************************************************************************************************************/
double difference(cv::Mat frame1, cv::Mat frame2) 
{
    cv::Mat grayA, medianA, grayB, medianB;
    cv::cvtColor(frame1, grayA, cv::COLOR_BGR2GRAY);
    cv::medianBlur(grayA, medianA, 5);
    cv::cvtColor(frame2, grayB, cv::COLOR_BGR2GRAY);
    cv::medianBlur(grayB, medianB, 5);

    cv::Mat diff;
    double score = cv::compareStructuralSimilarity(medianA, medianB, diff);
    diff = diff * 255;
    diff.convertTo(diff, CV_8U);

    return score;
}
/********************************************************************************************************************************/
int main() 
{
    cv::VideoCapture video("Test.mp4");
    if (!video.isOpened()) 
    {
        std::cout << "Error opening video file" << std::endl;
        return -1;
    }

    int length = video.get(cv::CAP_PROP_FRAME_COUNT);
    int codec = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    int fps = video.get(cv::CAP_PROP_FPS);
    int cap_width = video.get(cv::CAP_PROP_FRAME_WIDTH);
    int cap_height = video.get(cv::CAP_PROP_FRAME_HEIGHT);

    cv::Mat first_frame;
    video.read(first_frame);
    if (first_frame.empty()) 
    {
        std::cout << "Error reading first frame" << std::endl;
        return -1;
    }

    int x, y, width, height;
    cv::namedWindow("Select ROI", cv::WINDOW_NORMAL);
    cv::resizeWindow("Select ROI", 640, 480);
    cv::imshow("Select ROI", first_frame);
	
    /*********************************************************************************************
    mouse callback function for a window named "Select ROI". 
    The callback function is defined as a lambda function, which takes five parameters: event, x-coordinate, y-coordinate, flags, and userdata. 
    Inside the callback function, it checks if the event is a left button down event (EVENT_LBUTTONDOWN). 
    If it is, it casts the userdata pointer to a pointer of type cv::Rect and assigns the x and y coordinates 
    of the rectangle to the x_ and y_ values received from the callback function. 
		
    The userdata parameter is a pointer to a cv::Rect object, which represents a rectangular region of interest (ROI). 
    The cv::Rect object is created with the initial values of x, y, width, and height. 
    In summary, when the left mouse button is clicked inside the "Select ROI" window, 
    the x and y coordinates of the click are stored in the cv::Rect object, representing the selected region of interest.	
    ***********************************************************************************************/
    cv::setMouseCallback("Select ROI", [](int event, int x_, int y_, int flags, void* userdata) 
    {
        if (event == cv::EVENT_LBUTTONDOWN) 
	{
            cv::Rect* roi = static_cast<cv::Rect*>(userdata);
            roi->x = x_;
            roi->y = y_;
        }
    }, &cv::Rect(x, y, width, height));
    cv::waitKey(0);
    cv::destroyWindow("Select ROI");

    cv::Rect roi(x, y, width, height);
    cv::Mat roi_new;

    cv::VideoWriter output("test1.avi", codec, fps, cv::Size(cap_width, cap_height), true);
    if (!output.isOpened()) 
	{
        std::cout << "Error opening output video file" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) 
    {
        video.read(frame);
        if (frame.empty()) 
	{
            break;
        }

        roi_new = frame(roi);

        double similarity = difference(roi, roi_new);
        if (similarity < 0.8) 
	{
            cv::rectangle(frame, roi, cv::Scalar(0, 0, 255), 2);
            Beep(500, 200);
        }

        cv::imshow("new_frame", frame);
        output.write(frame);
        cv::waitKey(10);
    }

    output.release();
    video.release();
    cv::destroyAllWindows();

    return 0;
}
