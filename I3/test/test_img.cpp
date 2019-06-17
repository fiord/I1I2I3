#include <opencv2/opencv.hpp>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#define HEIGHT 480.0
#define WIDTH 640.0

int main() {
  cv::VideoCapture cap(0);
  assert(cap.isOpened());

  cv::Mat img;

  while (true) {
    assert(cap.read(img));

    // 640x480にリサイズ
    int width = img.rows;
    int height = img.cols;
    cv::Mat send_img;
    cv::resize(img, send_img, cv::Size(), WIDTH / width, HEIGHT / height);

    cv::imshow("tvphone", img);

    if (cv::waitKey(1) == 'q')  break;
  }

  cv::destroyAllWindows();
  return 0;

}

