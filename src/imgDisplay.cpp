/*
 * imgDisplay.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Read an image from a file and display it until user quits 'q'
 */

#include "opencv2/opencv.hpp" // general openCV header file

#include <cstdio>  // standard io functions and allocation
#include <cstring> // string functions

int main(int argc, char* argv[]) {
  cv::Mat src;
  char filename[256];

  // usage: check if the user provided a filename
  if (argc < 2) {
    printf("Usage: %s <image filename>\n", argv[0]);
    return -1;
  }

  // store the filename
  strcpy(filename, "../data/");
  strcat(filename, argv[1]);

  // read the image
  src = cv::imread(filename);

  // check if the image was successfully read
  if (src.data == NULL) {
    printf("unable to read the image %s\n", filename);
    return -2;
  }

  const char* windowName = "Image Display";
  cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

  while (true) {
    // display image
    cv::imshow(windowName, src);

    char key = cv::waitKey(10);

    // exit on 'q'
    if (key == 'q') {
      break;
    }
  }

  cv::destroyWindow(filename);

  return (0);
}