/*******************************************************************************************************************************************************
Reference: https://stackoverflow.com/questions/67699332/python-and-opencv-theft-detection-program
********************************************************************************************************************************************************/
#include <opencv2/opencv.hpp>
#include <vector>

cv::Scalar font = cv::FONT_HERSHEY_SIMPLEX;
/********************************************************************************************************************************************************/
cv::Mat combineWindow(float scale, std::vector < std::vector < cv::Mat >> & imgArray, std::vector < std::vector < std::string >> labs = {}) {
  int sizeW = imgArray[0][0].cols;
  int sizeH = imgArray[0][0].rows;
  int rows = imgArray.size();
  int cols = imgArray[0].size();
  bool rowsAvailable = typeid(imgArray[0]) == typeid(std::vector < cv::Mat > );
  int width = imgArray[0][0].cols;
  int height = imgArray[0][0].rows;
  if (rowsAvailable) {
    for (int x = 0; x < rows; x++) 
    {
      for (int y = 0; y < cols; y++) 
      {
        cv::resize(imgArray[x][y], imgArray[x][y], cv::Size(sizeW, sizeH), scale, scale);
        if (imgArray[x][y].channels() == 1) cv::cvtColor(imgArray[x][y], imgArray[x][y], cv::COLOR_GRAY2BGR);
      }
    }
    cv::Mat imageBlank(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
    std::vector < cv::Mat > hor(rows, imageBlank);
    std::vector < cv::Mat > hor_con(rows, imageBlank);
    for (int x = 0; x < rows; x++) {
      cv::hconcat(imgArray[x], hor[x]);
      cv::vconcat(hor[x], hor_con[x]);
    }
    cv::Mat ver;
    cv::vconcat(hor_con, ver);
    cv::Mat ver_con;
    cv::vconcat(hor_con, ver_con);
    return ver;
  } 
  else 
  {
    for (int x = 0; x < rows; x++) 
    {
      cv::resize(imgArray[x], imgArray[x], cv::Size(sizeW, sizeH), scale, scale);
      if (imgArray[x].channels() == 1) cv::cvtColor(imgArray[x], imgArray[x], cv::COLOR_GRAY2BGR);
    }
    cv::Mat hor;
    cv::hconcat(imgArray, hor);
    cv::Mat hor_con;
    cv::hconcat(imgArray, hor_con);
    cv::Mat ver = hor;
    if (labs.size() != 0) {
      int eachImgWidth = ver.cols / cols;
      int eachImgHeight = ver.rows / rows;
      for (int d = 0; d < rows; d++) 
      {
        for (int c = 0; c < cols; c++) 
	{
          cv::rectangle(ver, cv::Point(c * eachImgWidth, eachImgHeight * d), cv::Point(c * eachImgWidth + labs[d][c].length() * 13 + 27, 30 + eachImgHeight * d), cv::Scalar(255, 255, 255), cv::FILLED);
          cv::putText(ver, labs[d][c], cv::Point(eachImgWidth * c + 10, eachImgHeight * d + 20), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(255, 0, 255), 2);
        }
      }
    }
    return ver;
  }
}
/********************************************************************************************************************************************************/
cv::Mat checkFrame(cv::Mat referenceFrame, cv::Mat frame) 
{
  cv::Mat gray_frame;
  cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
  cv::Mat blur_frame;
  cv::GaussianBlur(gray_frame, blur_frame, cv::Size(15, 15), 0);
  cv::Mat difference;
  cv::absdiff(blur_frame, referenceFrameBlur, difference);
  cv::Mat thresh;
  cv::threshold(difference, thresh, 25, 255, cv::THRESH_BINARY);
  std::vector < std::vector < cv::Point >> contours;
  cv::findContours(thresh, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
  for (auto contour: contours) 
  {
    cv::Rect rect = cv::boundingRect(contour);
    if (cv::contourArea(contour) > 1200) 
    {
      cv::rectangle(thresh, rect, cv::Scalar(0, 255, 255), 2);
      cv::putText(thresh, "Intruder alert", cv::Point(50, 200), font, 2, cv::Scalar(0, 0, 255), 2);
    }
  }
  return thresh;
}
/********************************************************************************************************************************************************/
int main() 
{
  cv::VideoCapture capture(0, cv::CAP_DSHOW);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 360);
  capture.set(cv::CAP_PROP_FRAME_WIDTH, 480);
  cv::Mat frame;
  capture.read(frame);
  cv::Mat referenceFrame = frame.clone();
  cv::Mat referenceFrameGray;
  cv::Mat referenceFrameBlur;
  bool running = true;
  while (true) 
  {
    capture.read(frame);
    cv::cvtColor(referenceFrame, referenceFrameGray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(referenceFrameGray, referenceFrameBlur, cv::Size(15, 15), 0);
    cv::Mat thresh = checkFrame(referenceFrame, frame);
    cv::putText(referenceFrame, "Reference frame", cv::Point(0, 50), font, 1, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, "Live feed", cv::Point(0, 50), font, 1, cv::Scalar(0, 255, 0), 2);
    cv::putText(difference, "Frame difference", cv::Point(0, 50), font, 1, cv::Scalar(0, 255, 0), 2);
    cv::putText(thresh, "Binary difference", cv::Point(0, 50), font, 1, cv::Scalar(0, 255, 0), 2);
    cv::Mat combinedOutput = combineWindow(0.8, {
      {
        referenceFrame,
        frame
      },
      {
        difference,
        thresh
      }
    });
    cv::imshow("Theft Alert", combinedOutput);
    int key = cv::waitKey(1);
    if (key == '0') 
    {
      referenceFrame = frame.clone();
      cv::cvtColor(referenceFrame, referenceFrameGray, cv::COLOR_BGR2GRAY);
      cv::GaussianBlur(referenceFrameGray, referenceFrameBlur, cv::Size(15, 15), 0);
    }
    if (key == 'q' || key == 'Q') 
    {
      break;
    }
  }
  capture.release();
  cv::destroyAllWindows();
  return 0;
}
/********************************************************************************************************************************************************/
