/*
 * filters.cpp
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Implement custom image filters for Project 1
 */

#include "filters.h"

#include <cmath>

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
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_8UC3)
 * Returns:
 *  - 0 on success, -1 on error
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
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_8UC3)
 * Returns:
 *  - 0 on success, -1 on error
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

/*
 * sobelX3x3
 * Computes Sobel X (positive right) using separable filters:
 *  vertical [1 2 1]^T then horizontal [-1 0 1]
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_16SC3)
 * Returns:
 *  - 0 on success, -1 on error
 */
int sobelX3x3(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3)
    return -1;

  cv::Mat temp(src.size(), CV_16SC3);
  dst.create(src.size(), CV_16SC3);
  dst.setTo(cv::Scalar(0, 0, 0));

  // Vertical smoothing: temp(r,c) = src(r-1,c) + 2*src(r,c) + src(r+1,c)
  for (int r = 1; r < src.rows - 1; r++) {
    const cv::Vec3b* up = src.ptr<cv::Vec3b>(r - 1);
    const cv::Vec3b* cp = src.ptr<cv::Vec3b>(r);
    const cv::Vec3b* dn = src.ptr<cv::Vec3b>(r + 1);
    cv::Vec3s* tp = temp.ptr<cv::Vec3s>(r);

    for (int c = 0; c < src.cols; c++) {
      for (int ch = 0; ch < 3; ch++) {
        tp[c][ch] = (short)(up[c][ch] + 2 * cp[c][ch] + dn[c][ch]); // max 1020
      }
    }
  }

  // Horizontal derivative: dst(r,c) = -temp(r,c-1) + temp(r,c+1)
  for (int r = 1; r < src.rows - 1; r++) {
    const cv::Vec3s* tp = temp.ptr<cv::Vec3s>(r);
    cv::Vec3s* dp = dst.ptr<cv::Vec3s>(r);

    for (int c = 1; c < src.cols - 1; c++) {
      for (int ch = 0; ch < 3; ch++) {
        dp[c][ch] = (short)(-tp[c - 1][ch] + tp[c + 1][ch]); // range [-2040,2040]
      }
    }
  }

  return 0;
}

/*
 * sobelY3x3
 * Computes Sobel Y (positive up) using separable filters:
 *  horizontal [1 2 1] then vertical [1 0 -1]^T  (up - down)
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_16SC3)
 * Returns:
 *  - 0 on success, -1 on error
 */
int sobelY3x3(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3)
    return -1;

  cv::Mat temp(src.size(), CV_16SC3);
  dst.create(src.size(), CV_16SC3);
  dst.setTo(cv::Scalar(0, 0, 0));

  // Horizontal smoothing: temp(r,c) = src(r,c-1) + 2*src(r,c) + src(r,c+1)
  for (int r = 0; r < src.rows; r++) {
    const cv::Vec3b* sp = src.ptr<cv::Vec3b>(r);
    cv::Vec3s* tp = temp.ptr<cv::Vec3s>(r);

    for (int c = 1; c < src.cols - 1; c++) {
      for (int ch = 0; ch < 3; ch++) {
        tp[c][ch] = (short)(sp[c - 1][ch] + 2 * sp[c][ch] + sp[c + 1][ch]); // max 1020
      }
    }
  }

  // Vertical derivative (positive up): dst(r,c) = temp(r-1,c) - temp(r+1,c)
  for (int r = 1; r < src.rows - 1; r++) {
    const cv::Vec3s* up = temp.ptr<cv::Vec3s>(r - 1);
    const cv::Vec3s* dn = temp.ptr<cv::Vec3s>(r + 1);
    cv::Vec3s* dp = dst.ptr<cv::Vec3s>(r);

    for (int c = 1; c < src.cols - 1; c++) {
      for (int ch = 0; ch < 3; ch++) {
        dp[c][ch] = (short)(up[c][ch] - dn[c][ch]); // range [-2040,2040]
      }
    }
  }

  return 0;
}

/*
 * magnitude
 * Computes gradient magnitude image from Sobel X and Sobel Y images.
 * Arguments:
 * - sx: CV_16SC3 Sobel X result
 * - sy: CV_16SC3 Sobel Y result
 * - dst: output CV_8UC3 magnitude image
 * Returns:
 * - 0 on success, -1 on error
 */
int magnitude(cv::Mat& sx, cv::Mat& sy, cv::Mat& dst) {

  if (sx.empty() || sy.empty())
    return -1;
  if (sx.size() != sy.size())
    return -1;
  if (sx.type() != CV_16SC3 || sy.type() != CV_16SC3)
    return -1;

  dst.create(sx.size(), CV_8UC3);

  for (int r = 0; r < sx.rows; r++) {
    const cv::Vec3s* xptr = sx.ptr<cv::Vec3s>(r);
    const cv::Vec3s* yptr = sy.ptr<cv::Vec3s>(r);
    cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < sx.cols; c++) {
      for (int ch = 0; ch < 3; ch++) {
        int x = (int)xptr[c][ch];
        int y = (int)yptr[c][ch];

        int mag = (int)std::sqrt((double)(x * x + y * y));
        if (mag > 255)
          mag = 255;

        dptr[c][ch] = (unsigned char)mag;
      }
    }
  }

  return 0;
}

/*
 * blurQuantize
 * Blurs the image, then quantizes each color channel into 'levels' buckets.
 * Uses blur5x5_2 for the blur step.
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_8UC3)
 *  - levels: number of quantization levels per channel (e.g., 10)
 * Returns:
 *  - 0 on success, -1 on error
 */
int blurQuantize(cv::Mat& src, cv::Mat& dst, int levels) {

  if (src.empty() || src.type() != CV_8UC3)
    return -1;
  if (levels <= 0)
    return -1;

  cv::Mat blurred;
  if (blur5x5_2(src, blurred) != 0)
    return -1;

  dst.create(src.size(), src.type());

  int b = 255 / levels;
  if (b < 1)
    b = 1; // avoid divide-by-zero if levels > 255

  for (int r = 0; r < blurred.rows; r++) {
    const cv::Vec3b* sp = blurred.ptr<cv::Vec3b>(r);
    cv::Vec3b* dp = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < blurred.cols; c++) {
      for (int ch = 0; ch < 3; ch++) {
        int x = sp[c][ch];
        int xt = x / b;
        int xf = xt * b;
        if (xf > 255)
          xf = 255;
        dp[c][ch] = (unsigned char)xf;
      }
    }
  }

  return 0;
}

/*
 * depthFog
 * Adds fog based on depth values (0..255). Higher depth => more fog.
 * Arguments:
 * - src: input BGR image (CV_8UC3)
 * - depth8: TODO (CV_8UC1)
 * - dst: output BGR image (CV_8UC3)
 * Returns:
 *  - 0 on success, -1 on error
 */
int depthFog(cv::Mat& src, cv::Mat& depth8, cv::Mat& dst) {
  if (src.empty() || depth8.empty())
    return -1;
  if (src.type() != CV_8UC3 || depth8.type() != CV_8UC1)
    return -1;
  if (src.size() != depth8.size())
    return -1;

  dst.create(src.size(), src.type());

  // Fog color (light gray)
  const float fogB = 220.0f, fogG = 220.0f, fogR = 220.0f;

  for (int r = 0; r < src.rows; r++) {
    const cv::Vec3b* sp = src.ptr<cv::Vec3b>(r);
    const unsigned char* zp = depth8.ptr<unsigned char>(r);
    cv::Vec3b* dp = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < src.cols; c++) {
      float a = 1.0f - (zp[c] / 255.0f); // 0 near -> 1 far
      a = a * a;                         // stronger fog

      dp[c][0] = (unsigned char)((1.0f - a) * sp[c][0] + a * fogB);
      dp[c][1] = (unsigned char)((1.0f - a) * sp[c][1] + a * fogG);
      dp[c][2] = (unsigned char)((1.0f - a) * sp[c][2] + a * fogR);
    }
  }

  return 0;
}

/**
 * negative
 * Produces a negative of the input image by inverting each color channel.
 * Arguments:
 *  - src: input BGR image (CV_8UC3)
 *  - dst: output BGR image (CV_8UC3)
 * Returns:
 *  - 0 on success, -1 on error
 */
int negative(cv::Mat& src, cv::Mat& dst) {
  if (src.empty() || src.type() != CV_8UC3)
    return -1;
  dst.create(src.size(), src.type());

  for (int r = 0; r < src.rows; r++) {
    const cv::Vec3b* sp = src.ptr<cv::Vec3b>(r);
    cv::Vec3b* dp = dst.ptr<cv::Vec3b>(r);
    for (int c = 0; c < src.cols; c++) {
      dp[c][0] = 255 - sp[c][0];
      dp[c][1] = 255 - sp[c][1];
      dp[c][2] = 255 - sp[c][2];
    }
  }
  return 0;
}

/*
 * emboss
 * Creates an embossing effect by combining Sobel X and Sobel Y responses.
 * Uses a directional dot product and adds a bias so the result is visible.
 * Arguments:
 * - src: input BGR image (CV_8UC3)
 * - dst: output embossed image (CV_8UC3)
 * Returns:
 * - 0 on success, -1 on error
 */
int emboss(cv::Mat& src, cv::Mat& dst) {

  if (src.empty() || src.type() != CV_8UC3) {
    return -1;
  }

  cv::Mat sx, sy;

  // Compute Sobel gradients
  sobelX3x3(src, sx);
  sobelY3x3(src, sy);

  dst.create(src.size(), CV_8UC3);

  // Direction vector for embossing (diagonal light)
  const float dx = 0.7071f;
  const float dy = 0.7071f;

  for (int r = 0; r < src.rows; r++) {
    const cv::Vec3s* xptr = sx.ptr<cv::Vec3s>(r);
    const cv::Vec3s* yptr = sy.ptr<cv::Vec3s>(r);
    cv::Vec3b* dp = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < src.cols; c++) {
      for (int ch = 0; ch < 3; ch++) {

        // Dot product of gradient with direction
        float v = dx * xptr[c][ch] + dy * yptr[c][ch];

        // Scale and bias into visible range
        int out = (int)(v / 8.0f + 128.0f);
        if (out < 0)
          out = 0;
        if (out > 255)
          out = 255;

        dp[c][ch] = (unsigned char)out;
      }
    }
  }

  return 0;
}

/*
 * sepiaVignette
 * Applies a sepia tone filter with an added vignetting effect.
 * The sepia tone is computed using the original BGR values,
 * and vignetting darkens pixels as a function of distance from
 * the image center to mimic antique camera lens shading.
 *
 * Arguments:
 * - src: input image (CV_8UC3, BGR color)
 * - dst: output image (CV_8UC3, sepia + vignette)
 *
 * Returns:
 * - 0 on success, -1 on error
 */
int sepiaVignette(cv::Mat& src, cv::Mat& dst) {
  if (src.empty() || src.type() != CV_8UC3)
    return -1;

  dst.create(src.size(), src.type());

  float cx = (src.cols - 1) / 2.0f;
  float cy = (src.rows - 1) / 2.0f;
  float maxDist = std::sqrt(cx * cx + cy * cy);

  for (int r = 0; r < src.rows; r++) {
    const cv::Vec3b* sp = src.ptr<cv::Vec3b>(r);
    cv::Vec3b* dp = dst.ptr<cv::Vec3b>(r);

    for (int c = 0; c < src.cols; c++) {
      // original BGR
      unsigned char B = sp[c][0];
      unsigned char G = sp[c][1];
      unsigned char R = sp[c][2];

      // sepia from ORIGINAL channels
      int newB = (int)(0.272 * R + 0.534 * G + 0.131 * B);
      int newG = (int)(0.349 * R + 0.686 * G + 0.168 * B);
      int newR = (int)(0.393 * R + 0.769 * G + 0.189 * B);

      if (newB > 255)
        newB = 255;
      if (newG > 255)
        newG = 255;
      if (newR > 255)
        newR = 255;

      // vignette factor: 1 at center -> ~minFactor at corners
      float dx = c - cx;
      float dy = r - cy;
      float d = std::sqrt(dx * dx + dy * dy) / maxDist; // 0..1

      // Smooth curve; tweak these two constants for stronger/weaker vignette
      const float minFactor = 0.35f; // edge darkness (0.0 darker, 1.0 none)
      float t = d * d;               // quadratic falloff
      float factor = 1.0f - t * (1.0f - minFactor);

      // apply vignette to sepia result
      newB = (int)(newB * factor);
      newG = (int)(newG * factor);
      newR = (int)(newR * factor);

      dp[c][0] = (unsigned char)newB;
      dp[c][1] = (unsigned char)newG;
      dp[c][2] = (unsigned char)newR;
    }
  }

  return 0;
}