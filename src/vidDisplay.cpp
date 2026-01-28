/*
 * vidDisplay.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Display live video feed with keypress features:
 *          - 'g' = OpenCV grayscale
 *          - 'h' = custom grayscale
 *          - 'p' = sepia filter
 *          - 'b' = 5x5 blur
 *          - 'x' / 'y' = X Sobel / Y Sobel
 *          - 'r' = reset filters
 *          - 's' = save
 *          - 'q' = quit
 */

#include "DA2Network.hpp"
#include "faceDetect.h"
#include "filters.h"          // face detector
#include "opencv2/opencv.hpp" // general openCV header file

#include <cstdio> // standard io functions and allocation
#include <ctime>

enum DisplayMode {
  MODE_COLOR,
  MODE_OPENCV_GRAY,
  MODE_CUSTOM_GRAY,
  MODE_SEPIA,
  MODE_BLUR,
  MODE_SOBEL_X,
  MODE_SOBEL_Y,
  MODE_MAG,
  MODE_BLUR_QUANT,
  MODE_FACE,
  MODE_DEPTH,
  MODE_DEPTH_FOG,
  MODE_NEG,
  MODE_EMBOSS
};

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
  cv::Mat gray;
  cv::Mat display;
  cv::Mat sx16, sy16;
  cv::Mat vis8;
  std::vector<cv::Rect> faces;

  // depth
  const float reduction = 0.5f;
  DA2Network da_net("../data/model_fp16.onnx");

  float scale_factor = 256.0f / (refS.height * reduction);
  printf("Using DA2 scale factor %.2f\n", scale_factor);

  cv::Mat depth8;    // CV_8UC1
  cv::Mat depth_vis; // CV_8UC3
  cv::Mat small;     // reduced frame (CV_8UC3)

  // default mode
  DisplayMode mode = MODE_COLOR;

  for (;;) {
    capdev >> frame; // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }

    bool needDepth = (mode == MODE_DEPTH || mode == MODE_DEPTH_FOG);

    if (needDepth) {
      // reduce input frame for speed (matches sample)
      cv::resize(frame, small, cv::Size(), reduction, reduction);

      da_net.set_input(small, scale_factor);
      da_net.run_network(depth8, small.size()); // depth8: CV_8UC1

      if (mode == MODE_DEPTH) {
        cv::applyColorMap(depth8, depth_vis, cv::COLORMAP_INFERNO);
        display = depth_vis; // note: display size is reduced
      } else {               // MODE_DEPTH_FOG
        if (depthFog(small, depth8, display) != 0) {
          display = small;
        }
      }
    }

    if (mode == MODE_COLOR) {
      display = frame;
    } else if (mode == MODE_OPENCV_GRAY) {
      // convert BGR -> Gray
      cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
      // convert Gray -> BGR so display is still 3-channel
      cv::cvtColor(gray, display, cv::COLOR_GRAY2BGR);
    } else if (mode == MODE_CUSTOM_GRAY) {
      if (grayscale(frame, display) != 0) {
        display = frame;
      }
    } else if (mode == MODE_SEPIA) {
      if (sepia(frame, display) != 0) {
        display = frame;
      }
    } else if (mode == MODE_BLUR) {
      //   if (blur5x5_1(frame, display) != 0) {
      if (blur5x5_2(frame, display) != 0) {
        display = frame;
      }
    } else if (mode == MODE_SOBEL_X) {
      if (sobelX3x3(frame, sx16) == 0) {
        cv::convertScaleAbs(sx16, vis8);
        display = vis8;
      } else {
        display = frame;
      }
    } else if (mode == MODE_SOBEL_Y) {
      if (sobelY3x3(frame, sy16) == 0) {
        cv::convertScaleAbs(sy16, vis8);
        display = vis8;
      } else {
        display = frame;
      }
    } else if (mode == MODE_MAG) {
      if (sobelX3x3(frame, sx16) == 0 && sobelY3x3(frame, sy16) == 0) {
        magnitude(sx16, sy16, display); // display becomes CV_8UC3
      } else {
        display = frame;
      }
    } else if (mode == MODE_BLUR_QUANT) {
      if (blurQuantize(frame, display, 10) != 0) {
        display = frame;
      }
    } else if (mode == MODE_FACE) {

      display = frame.clone(); // draw on a copy

      cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
      detectFaces(gray, faces);
      drawBoxes(display, faces, 30, 1.0);
    } else if (mode == MODE_NEG) {
      if (negative(frame, display) != 0) {
        display = frame;
      }
    } else if (mode == MODE_EMBOSS) {
      if (emboss(frame, display) != 0) {
        display = frame;
      }
    }

    cv::imshow(windowName, display);

    // see if there is a waiting keystroke
    char key = (char)cv::waitKey(10);
    if (key == 'q') {
      break;
    } else if (key == 's') {
      // timestamped filename
      std::time_t t = std::time(nullptr);
      char outname[256];
      std::snprintf(outname, sizeof(outname), "../out/frame_%ld.png", (long)t);

      if (cv::imwrite(outname, display)) {
        printf("Saved: %s\n", outname);
      } else {
        printf("Failed to save: %s\n", outname);
      }
    } else if (key == 'r') { // reset display
      mode = MODE_COLOR;
    } else if (key == 'g') { // OpenCV grayscale
      mode = MODE_OPENCV_GRAY;
    } else if (key == 'h') { // custom grayscale
      mode = MODE_CUSTOM_GRAY;
    } else if (key == 'p') { // Sepia tone filter
      mode = MODE_SEPIA;
    } else if (key == 'b') { // 5x5 blur filter
      mode = MODE_BLUR;
    } else if (key == 'x') { // 3x3 Sobel X
      mode = MODE_SOBEL_X;
    } else if (key == 'y') { // 3x3 Sobel Y
      mode = MODE_SOBEL_Y;
    } else if (key == 'm') { // Gradient Magnitude
      mode = MODE_MAG;
    } else if (key == 'l') { // blur and quantize
      mode = MODE_BLUR_QUANT;
    } else if (key == 'f') { // face detection
      mode = MODE_FACE;
    } else if (key == 'd') { // depth
      mode = MODE_DEPTH;
    } else if (key == 'o') { // depth fog
      mode = MODE_DEPTH_FOG;
    } else if (key == 'n') { // negative
      mode = MODE_NEG;
    } else if (key == 'e') { // emboss
      mode = MODE_EMBOSS;
    }
  }

  cv::destroyWindow(windowName);
  return 0;
}