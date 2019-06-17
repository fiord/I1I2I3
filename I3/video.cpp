#include "video.hpp"
#include "connect.hpp"

#define HEIGHT 480.0
#define WIDTH 640.0

void send_recv(int s) {
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    die("failed to open camera\n");
  }

  cv::Mat img;

  while (true) {
    if (!cap.read(img)) {
      die("failed to read image data\n");
    }

    // 640x480にリサイズ
    int width = img.rows;
    int height = img.cols;
    cv::Mat send_img;
    cv::resize(img, send_img, cv::Size(), WIDTH / width, HEIGHT / height);

    // 送信 
    int m = send(s, &send_img, sizeof(send_img), 0);
    if (m != sizeof(send_img))  die("failed to send img data\n");

    // 受信・表示
    m = recv(s, &img, sizeof(send_img), 0);
    cv::imshow("tvphone", img);

    if (cv::waitKey(1) == 'q')  break;
  }

  cv::destroyAllWindows();

}

void video_server(char *port) {
  int s = start_server(port);

  send_recv(s);
}

void video_client(char *ip, char *port) {
  int s = connect_server(ip, port);

  send_recv(s);
}
