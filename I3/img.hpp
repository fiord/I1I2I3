#ifndef INCLUDE_GUARD_IMG_HPP
#define INCLUDE_GUARD_IMG_HPP
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <cstdlib>
#include "die.hpp"

cv::VideoCapture init() {
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    die("failed to open camera");
  }

  return cap;
}

void get_image(cv::VideoCapture &cap, cv::Mat &image) {
  if (cap.read(image)) {
    return;
  }
  die("failed to read image");
}

void print_image(cv::Mat &img) {
  cv::imshow("tvphone", img);
}

bool check_key() {
  const int key = cv::waitKey(1);
  return key == 'q';
}
#endif // INCLUDE_GUARD_IMG_HPP
