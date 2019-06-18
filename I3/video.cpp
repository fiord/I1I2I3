#include "video.hpp"
#include "connect.hpp"

#define PACKET_SIZE 65500
#define HEIGHT 480
#define WIDTH 640

void send_video(int s) {
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    die("failed to open camera");
  } 
  
  cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

  cv::Mat img;
  std::vector<unsigned char> ibuff;
  std::vector<int> param = { CV_IMWRITE_JPEG_QUALITY, 85 };
  char buff[PACKET_SIZE];

  while(1) {
    if (!cap.read(img)) {
      die("failed to read image data");
    }

    cv::imencode(".jpg", img, ibuff, param);
    if (ibuff.size() < PACKET_SIZE) {
      for (int i = 0; i < PACKET_SIZE; i++) {
        if (i < ibuff.size()) buff[i] = ibuff[i];
        else  buff[i] = 0;
      }
      int m = send(s, buff, PACKET_SIZE, 0);
      if (m != PACKET_SIZE)  die("failed to send img data");
    }
    ibuff.clear();
  }
}

void recv_video(int s) {
  cvNamedWindow("tvphone", CV_WINDOW_AUTOSIZE);

  cv::Mat img = cv::Mat(HEIGHT, WIDTH, CV_8UC3);
  char buff[PACKET_SIZE];
  std::vector<unsigned char> ibuff;

  while(1) {
    // 受信・表示
    int m = recv(s, buff, PACKET_SIZE, 0);
    fprintf(stderr, "[info] recv: recv_size=%d\n", m);
    for (int i = 0; i < sizeof(buff); i++) {
      ibuff.push_back((unsigned char)buff[i]);
    }
    if (m == -1)  break;
    img = cv::imdecode(cv::Mat(ibuff), CV_LOAD_IMAGE_COLOR);
    cv::imshow("tvphone", img);
    ibuff.clear();
  }
  cv::destroyAllWindows();
}

void send_recv_video(int s) {
  std::thread sender(send_video, s);
  std::thread recver(recv_video, s);

  sender.join();
  recver.join();
  close(s);
}

