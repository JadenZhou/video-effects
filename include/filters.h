/*
 * filters.h
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Function prototypes for image filters
 */

#ifndef FILTERS_H
#define FILTERS_H

#include <opencv2/opencv.hpp>

// Custom grayscale
int grayscale(cv::Mat& src, cv::Mat& dst);

// Sepia tone filter
int sepia(cv::Mat& src, cv::Mat& dst);

// Naive 5x5 blur filter
int blur5x5_1(cv::Mat& src, cv::Mat& dst);

// Faster 5x5 blur filter
int blur5x5_2(cv::Mat& src, cv::Mat& dst);

// 3x3 Sobel X filter
int sobelX3x3(cv::Mat& src, cv::Mat& dst);

// 3x3 Sobel Y filter
int sobelY3x3(cv::Mat& src, cv::Mat& dst);

// Gradient magnitude with Euclidean distance
int magnitude(cv::Mat& sx, cv::Mat& sy, cv::Mat& dst);

// Blurs and quantizes image to fixed number of levels
int blurQuantize(cv::Mat& src, cv::Mat& dst, int levels);

// Adds fog based on depth values
int depthFog(cv::Mat& src, cv::Mat& depth8, cv::Mat& dst);

#endif