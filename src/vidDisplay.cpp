/*
 * vidDisplay.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Display live video feed, quit on 'q', save frame on 's'
 */

#include "opencv2/opencv.hpp" // general openCV header file

#include <cstdio>  // standard io functions and allocation
#include <cstring> // string functions

/*
 * main
 * Opens the default camera and displays frames in a window.
 * - Press 'q' to quit
 * - Press 's' to save the current frame to ../out/
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

  for (;;) {
    *capdev >> frame; // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }
    cv::imshow(windowName, frame);

    // see if there is a waiting keystroke
    char key = cv::waitKey(10);
    if (key == 'q') {
      break;
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