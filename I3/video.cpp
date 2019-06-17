#include "video.hpp"
#include "connect.hpp"

#define HEIGHT 480.0
#define WIDTH 640.0

void send_recv(int s) {
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    die("failed to open camera");
  }

  cv::Mat img;

  while (true) {
    if (!cap.read(img)) {
      die("failed to read image data");
    }

    // 640x480にリサイズ
    int width = img.rows;
    int height = img.cols;
    cv::Mat send_img;
    cv::resize(img, send_img, cv::Size(), WIDTH / width, HEIGHT / height);

    // 送信 
    int m = send(s, &send_img, sizeof(send_img), 0);
    fprintf(stderr, "send: img_size=%d, send_size=%d\n", sizeof(send_img), m);
    if (m != sizeof(send_img))  die("failed to send img data");

    // 受信・表示
    m = recv(s, &send_img, sizeof(send_img), 0);
    fprintf(stderr, "recv: recv_size=%d\n", m);
    cv::imshow("tvphone", send_img);
  }

  cv::destroyAllWindows();

}

