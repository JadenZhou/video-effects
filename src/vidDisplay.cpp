/*
 * vidDisplay.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Display live video feed, quit on 'q', save frame on 's', toggle grayscale with 'g'
 */

#include "opencv2/opencv.hpp" // general openCV header file

#include <cstdio>  // standard io functions and allocation
#include <cstring> // string functions

/*
 * main
 * Opens the default camera and displays frames in a window.
 * - 'q' to quit
 * - 's' to save the current frame to ../out/
 * - 'g' toggles grayscale display
 */
int main(int argc, char* argv[]) {
  cv::VideoCapture* capdev;

  // open the video device
  capdev = new cv::VideoCapture(0);
  if (!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return (-1);
  }

  // get some properties of the image
  cv::Size refS((int)capdev->get(cv::CAP_PROP_FRAME_WIDTH),
                (int)capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
  printf("Expected size: %d %d\n", refS.width, refS.height);

  const char* windowName = "Video";
  cv::namedWindow(windowName, 1); // identifies a window

  cv::Mat frame;
  cv::Mat gray;
  cv::Mat display;

  bool grayscale = false; // store grayscale keypress choice

  for (;;) {
    *capdev >> frame; // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }

    if (grayscale) {
      // convert BGR -> Gray
      cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
      // convert Gray -> BGR so display is still 3-channel
      cv::cvtColor(gray, display, cv::COLOR_GRAY2BGR);
    } else {
      display = frame;
    }

    cv::imshow(windowName, frame);

    // see if there is a waiting keystroke
    char key = cv::waitKey(10);
    if (key == 'q') {
      break;
    } else if (key == 'g') { // toggle grayscale
      grayscale = true;
    } else if (key == 'r') { // reset display
      grayscale = false;
    } else if (key == 's') {
      // timestamped filename
      std::time_t t = std::time(nullptr);
      char outname[256];
      std::snprintf(outname, sizeof(outname), "../out/frame_%ld.png", (long)t);

      bool ok = cv::imwrite(outname, frame);
      if (ok) {
        printf("Saved: %s\n", outname);
      } else {
        printf("Failed to save: %s\n", outname);
      }
    }
  }

  delete capdev;
  return (0);
}