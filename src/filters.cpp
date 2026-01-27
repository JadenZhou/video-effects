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

/*
 * blur5x5_1
 * Naive 5x5 blur using the full 5x5 kernel (sum=100).
 * - src: CV_8UC3
 * - dst: CV_8UC3
 * Border handling: copy src to dst, overwrite interior only.
 */
int blur5x5_1(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3)
    return -1;

  dst = src.clone(); // copy borders + initialize

  static const int k[5][5] = {
      {1, 2, 4, 2, 1}, {2, 4, 8, 4, 2}, {4, 8, 16, 8, 4}, {2, 4, 8, 4, 2}, {1, 2, 4, 2, 1}};

  for (int r = 2; r < src.rows - 2; r++) {
    for (int c = 2; c < src.cols - 2; c++) {

      int sumB = 0, sumG = 0, sumR = 0;

      for (int dr = -2; dr <= 2; dr++) {
        for (int dc = -2; dc <= 2; dc++) {
          cv::Vec3b pix = src.at<cv::Vec3b>(r + dr, c + dc);
          int w = k[dr + 2][dc + 2];

          sumB += w * pix[0];
          sumG += w * pix[1];
          sumR += w * pix[2];
        }
      }

      dst.at<cv::Vec3b>(r, c)[0] = (unsigned char)(sumB / 100);
      dst.at<cv::Vec3b>(r, c)[1] = (unsigned char)(sumG / 100);
      dst.at<cv::Vec3b>(r, c)[2] = (unsigned char)(sumR / 100);
    }
  }

  return 0;
}

/*
 * blur5x5_2
 * Faster 5x5 blur using separable 1x5 filters and pointer access.
 * Uses temp CV_16SC3 to keep precision.
 * Border handling: copy src to dst, overwrite interior only.
 */
int blur5x5_2(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3)
    return -1;

  dst = src.clone(); // borders remain no-zero
  cv::Mat temp(src.size(), CV_16SC3);

  const int w0 = 1, w1 = 2, w2 = 4; // [1 2 4 2 1]

  // Horizontal pass: src (uchar) -> temp (short), skip 2-pixel border
  for (int r = 0; r < src.rows; r++) {
    const cv::Vec3b* sp = src.ptr<cv::Vec3b>(r);
    cv::Vec3s* tp = temp.ptr<cv::Vec3s>(r);

    // can leave tp[c] undefined on borders because won't be read
    for (int c = 2; c < src.cols - 2; c++) {
      for (int ch = 0; ch < 3; ch++) {
        int sum = w0 * sp[c - 2][ch] + w1 * sp[c - 1][ch] + w2 * sp[c][ch] + w1 * sp[c + 1][ch] +
                  w0 * sp[c + 2][ch];
        tp[c][ch] = (short)sum; // max 255*10 = 2550 fits in short
      }
    }
  }

  // Vertical pass: temp (short) -> dst (uchar), skip 2-pixel border
  for (int r = 2; r < src.rows - 2; r++) {
    const cv::Vec3s* t0 = temp.ptr<cv::Vec3s>(r - 2);
    const cv::Vec3s* t1 = temp.ptr<cv::Vec3s>(r - 1);
    const cv::Vec3s* t2 = temp.ptr<cv::Vec3s>(r);
    const cv::Vec3s* t3 = temp.ptr<cv::Vec3s>(r + 1);
    const cv::Vec3s* t4 = temp.ptr<cv::Vec3s>(r + 2);

    cv::Vec3b* dp = dst.ptr<cv::Vec3b>(r);

    for (int c = 2; c < src.cols - 2; c++) {
      for (int ch = 0; ch < 3; ch++) {
        int sum =
            w0 * t0[c][ch] + w1 * t1[c][ch] + w2 * t2[c][ch] + w1 * t3[c][ch] + w0 * t4[c][ch];

        // sum max: 2550*10 = 25500; final divide by 100
        int val = sum / 100;
        if (val < 0)
          val = 0;
        if (val > 255)
          val = 255;
        dp[c][ch] = (unsigned char)val;
      }
    }
  }

  return 0;
}