/*
 * imgEffects.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Apply Project 1 effects to a still image and allow saving results.
 *
 * Controls:
 * - 'g' = OpenCV grayscale
 * - 'h' = custom grayscale
 * - 'p' = sepia filter
 * - 'b' = 5x5 blur
 * - 'x' = X Sobel
 * - 'y' = Y Sobel
 * - 'm' = Gradient magnitude
 * - 'l' = blur quantize
 * - 'f' = face detection
 * - 'd' = depth
 * - 'o' = depth fog
 * - 'n' = negative
 * - 'e' = emboss
 * - 'u' = pixelate
 * - 'v' = sepia vignette
 * - 'c' = edge heatmap
 * - 'r' = reset filters
 * - 's' = save
 * - 'q' = quit
 */

#include "DA2Network.hpp"
#include "faceDetect.h"
#include "filters.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <vector>

enum Mode {
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
  MODE_EMBOSS,
  MODE_FACE_PIXELATE,
  MODE_SEPIA_VIGNETTE,
  MODE_EDGE_HEATMAP
};

static const char* modeName(Mode m) {
  switch (m) {
  case MODE_COLOR:
    return "color";
  case MODE_OPENCV_GRAY:
    return "gray";
  case MODE_CUSTOM_GRAY:
    return "customgray";
  case MODE_SEPIA:
    return "sepia";
  case MODE_BLUR:
    return "blur";
  case MODE_SOBEL_X:
    return "sobelx";
  case MODE_SOBEL_Y:
    return "sobely";
  case MODE_MAG:
    return "mag";
  case MODE_BLUR_QUANT:
    return "blurquant";
  case MODE_FACE:
    return "face";
  case MODE_NEG:
    return "neg";
  case MODE_EMBOSS:
    return "emboss";
  case MODE_DEPTH:
    return "depth";
  case MODE_DEPTH_FOG:
    return "depthfog";
  case MODE_FACE_PIXELATE:
    return "pixelate";
  case MODE_SEPIA_VIGNETTE:
    return "vignette";
  case MODE_EDGE_HEATMAP:
    return "edgeheatmap";
  default:
    return "out";
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <image filename>\n", argv[0]);
    return -1;
  }

  char filename[256];
  strcpy(filename, "../data/");
  strcat(filename, argv[1]);

  cv::Mat src = cv::imread(filename);
  if (src.empty()) {
    printf("Unable to read image %s\n", filename);
    return -1;
  }

  const char* win = "Still Effects";
  cv::namedWindow(win, 1);

  Mode mode = MODE_COLOR;

  // scratch mats
  cv::Mat gray1, display, sx16, sy16, vis8, mag8, mag1;
  std::vector<cv::Rect> faces;

  // Depth
  DA2Network da_net("../data/model_fp16.onnx");
  int minDim = (src.rows < src.cols) ? src.rows : src.cols;
  float scale_factor = 512.0f / (float)minDim;
  scale_factor = (scale_factor > 1.0f) ? 1.0f : scale_factor;
  cv::Mat depth8, depth_vis;

  while (true) {
    // build display from mode
    if (mode == MODE_COLOR) {
      display = src.clone();

    } else if (mode == MODE_OPENCV_GRAY) {
      cv::cvtColor(src, gray1, cv::COLOR_BGR2GRAY);
      cv::cvtColor(gray1, display, cv::COLOR_GRAY2BGR);

    } else if (mode == MODE_CUSTOM_GRAY) {
      if (grayscale(src, display) != 0)
        display = src.clone();

    } else if (mode == MODE_SEPIA) {
      if (sepia(src, display) != 0)
        display = src.clone();

    } else if (mode == MODE_BLUR) {
      if (blur5x5_2(src, display) != 0)
        display = src.clone();

    } else if (mode == MODE_SOBEL_X) {
      if (sobelX3x3(src, sx16) == 0) {
        cv::convertScaleAbs(sx16, display);
      } else
        display = src.clone();

    } else if (mode == MODE_SOBEL_Y) {
      if (sobelY3x3(src, sy16) == 0) {
        cv::convertScaleAbs(sy16, display);
      } else
        display = src.clone();

    } else if (mode == MODE_MAG) {
      if (sobelX3x3(src, sx16) == 0 && sobelY3x3(src, sy16) == 0) {
        magnitude(sx16, sy16, display);
      } else
        display = src.clone();

    } else if (mode == MODE_BLUR_QUANT) {
      if (blurQuantize(src, display, 10) != 0)
        display = src.clone();

    } else if (mode == MODE_FACE) {
      display = src.clone();

      cv::cvtColor(src, gray1, cv::COLOR_BGR2GRAY);
      detectFaces(gray1, faces);
      drawBoxes(display, faces, 30, 1.0);

    } else if (mode == MODE_NEG) {
      if (negative(src, display) != 0)
        display = src.clone();

    } else if (mode == MODE_EMBOSS) {
      if (emboss(src, display) != 0)
        display = src.clone();

    } else if (mode == MODE_DEPTH || mode == MODE_DEPTH_FOG) {
      da_net.set_input(src, scale_factor);
      da_net.run_network(depth8, src.size());

      if (mode == MODE_DEPTH) {
        cv::applyColorMap(depth8, depth_vis, cv::COLORMAP_INFERNO);
        display = depth_vis;
      } else {
        if (depthFog(src, depth8, display) != 0)
          display = src.clone();
      }

    } else if (mode == MODE_FACE_PIXELATE) {
      display = src.clone();

      cv::cvtColor(src, gray1, cv::COLOR_BGR2GRAY);
      detectFaces(gray1, faces);

      // Pixelation strength: bigger value => more pixelated
      const int pixelFactor = 10;

      for (const auto& f : faces) {

        // Clamp face rectangle to image bounds
        cv::Rect r = f & cv::Rect(0, 0, src.cols, src.rows);
        if (r.width <= 0 || r.height <= 0)
          continue;

        // Extract face ROI from the display image (so we only modify the displayed output)
        cv::Mat roi = display(r);

        // Compute small size (at least 1x1)
        int wSmall = r.width / pixelFactor;
        int hSmall = r.height / pixelFactor;
        if (wSmall < 1)
          wSmall = 1;
        if (hSmall < 1)
          hSmall = 1;

        cv::Mat small;

        // Downsample the ROI (area-based)
        cv::resize(roi, small, cv::Size(wSmall, hSmall), 0, 0, cv::INTER_AREA);

        // Upsample back to original ROI size using nearest-neighbor to keep blocky pixels
        cv::resize(small, roi, roi.size(), 0, 0, cv::INTER_NEAREST);
      }

    } else if (mode == MODE_SEPIA_VIGNETTE) {
      if (sepiaVignette(src, display) != 0)
        display = src.clone();
    } else if (mode == MODE_EDGE_HEATMAP) {
      if (sobelX3x3(src, sx16) == 0 && sobelY3x3(src, sy16) == 0 &&
          magnitude(sx16, sy16, mag8) == 0) {
        cv::cvtColor(mag8, mag1, cv::COLOR_BGR2GRAY);
        cv::applyColorMap(mag1, display, cv::COLORMAP_TURBO);
      }
    } else {
      display = src.clone();
    }

    // show + wait
    cv::imshow(win, display);
    char key = (char)cv::waitKey(0);

    if (key == 'q') {
      break;
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
    } else if (key == 'u') { // pixelate face
      mode = MODE_FACE_PIXELATE;
    } else if (key == 'v') { // sepia vignette
      mode = MODE_SEPIA_VIGNETTE;
    } else if (key == 'c') { // edge heatmap
      mode = MODE_EDGE_HEATMAP;
    } else if (key == 's') { // save
      std::time_t t = std::time(nullptr);
      char outname[256];
      std::snprintf(outname, sizeof(outname), "../out/effect_%s_%ld.png", modeName(mode), (long)t);

      if (cv::imwrite(outname, display)) {
        printf("Saved: %s\n", outname);
      } else {
        printf("Failed to save: %s\n", outname);
      }
    }
  }

  cv::destroyWindow(win);
  return 0;
}