/*
 * filters.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Implement custom image filters for Project 1
 */

#include "filters.h"

/*
 * grayscale
 * Converts a BGR color image to a custom grayscale image.
 *   gray = 255 - R
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_8UC3) with identical channels
 * Returns:
 *  - 0 on success, -1 on error
 */
int grayscale(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3) {
    return -1;
  }

  dst.create(src.size(), src.type());

  for (int r = 0; r < src.rows; r++) {
    cv::Vec3b* sptr = src.ptr<cv::Vec3b>(r);
    cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < src.cols; c++) {
      unsigned char B = sptr[c][0];
      unsigned char G = sptr[c][1];
      unsigned char R = sptr[c][2];

      (void)B;
      (void)G; // unused in this variant

      unsigned char gray = (unsigned char)(255 - R);

      dptr[c][0] = gray; // B
      dptr[c][1] = gray; // G
      dptr[c][2] = gray; // R
    }
  }

  return 0;
}

/*
 * sepia
 * Applies a sepia tone to a BGR image using the provided coefficient matrix.
 * Uses the original channel values for computations and clamps results to [0,255].
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_8UC3)
 * Returns:
 *  - 0 on success, -1 on error
 */
int sepia(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3) {
    return -1;
  }

  dst.create(src.size(), src.type());

  for (int r = 0; r < src.rows; r++) {
    cv::Vec3b* sptr = src.ptr<cv::Vec3b>(r);
    cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < src.cols; c++) {
      // OpenCV order: B, G, R
      unsigned char B = sptr[c][0];
      unsigned char G = sptr[c][1];
      unsigned char R = sptr[c][2];

      // Compute new RGB
      int newB = (int)(0.272 * R + 0.534 * G + 0.131 * B);
      int newG = (int)(0.349 * R + 0.686 * G + 0.168 * B);
      int newR = (int)(0.393 * R + 0.769 * G + 0.189 * B);

      // Clamp to 255
      if (newB > 255)
        newB = 255;
      if (newG > 255)
        newG = 255;
      if (newR > 255)
        newR = 255;

      dptr[c][0] = (unsigned char)newB;
      dptr[c][1] = (unsigned char)newG;
      dptr[c][2] = (unsigned char)newR;
    }
  }

  return 0;
}