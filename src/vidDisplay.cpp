/*
 * vidDisplay.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Display live video feed with keypress features:
 *          - 'q' = quit
 *          - 'g' = OpenCV grayscale
 *          - 'h' = custom grayscale
 *          - 's' = save
 *          - 'q' = quit
 */

#include "filters.h"
#include "opencv2/opencv.hpp" // general openCV header file

#include <cstdio> // standard io functions and allocation
#include <ctime>

enum DisplayMode { MODE_COLOR, MODE_OPENCV_GRAY, MODE_CUSTOM_GRAY, MODE_SEPIA };

int main(int argc, char* argv[]) {

  cv::VideoCapture capdev(0);
  if (!capdev.isOpened()) {
    printf("Unable to open video device\n");
    return -1;
  }

  // get some properties of the image
  cv::Size refS((int)capdev.get(cv::CAP_PROP_FRAME_WIDTH),
                (int)capdev.get(cv::CAP_PROP_FRAME_HEIGHT));
  printf("Expected size: %d %d\n", refS.width, refS.height);

  const char* windowName = "Video";
  cv::namedWindow(windowName, 1); // identifies a window

  cv::Mat frame;
  cv::Mat gray1;
  cv::Mat display;
  DisplayMode mode = MODE_COLOR;

  for (;;) {
    capdev >> frame; // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }

    if (mode == MODE_COLOR) {
      display = frame;
    } else if (mode == MODE_OPENCV_GRAY) {
      // convert BGR -> Gray
      cv::cvtColor(frame, gray1, cv::COLOR_BGR2GRAY);
      // convert Gray -> BGR so display is still 3-channel
      cv::cvtColor(gray1, display, cv::COLOR_GRAY2BGR);
    } else if (mode == MODE_CUSTOM_GRAY) {
      grayscale(frame, display);
    } else if (mode == MODE_SEPIA) {
      sepia(frame, display);
    }

    cv::imshow(windowName, frame);

    // see if there is a waiting keystroke
    char key = (char)cv::waitKey(10);
    if (key == 'q') {
      break;
    } else if (key == 'r') { // reset display
      mode = MODE_COLOR;
    } else if (key == 'g') { // OpenCV grayscale
      mode = MODE_OPENCV_GRAY;
    } else if (key == 'h') {
      mode = MODE_CUSTOM_GRAY;
    } else if (key == 'p') {
      mode = MODE_SEPIA;
    } else if (key == 's') {
      // timestamped filename
      std::time_t t = std::time(nullptr);
      char outname[256];
      std::snprintf(outname, sizeof(outname), "../out/frame_%ld.png", (long)t);

      bool ok = cv::imwrite(outname, display);
      if (ok) {
        printf("Saved: %s\n", outname);
      } else {
        printf("Failed to save: %s\n", outname);
      }
    }
  }

  cv::destroyWindow(windowName);
  return 0;
}