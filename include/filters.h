/*
 * filters.h
 * Name: Jaden Zhou
 * Date: Jan 2026
 * Purpose: Function prototypes for image filters
 */

#ifndef FILTERS_H
#define FILTERS_H

#include <opencv2/opencv.hpp>

// Custom grayscale (different from OpenCV cvtColor)
int grayscale(cv::Mat& src, cv::Mat& dst);

#endif