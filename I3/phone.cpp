#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits.h>
#include "die.hpp"
#include "connect.hpp"
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM
#define PACKET_SIZE 1024

#undef DEBUG

#include "voice.hpp"

#undef USE_VIDEO
#ifdef USE_VIDEO
#include "img.hpp"
#endif

int main(int argc, char **argv) {
  int s;
  if (argc == 2) {
    s = start_server(argv[1]);
  }
  else if(argc == 3) {
    s = connect_server(argv[1], argv[2]);
  }
  else {
    die("wrong usage\n");
  }
#ifdef USE_VIDEO
  cv::VideoCapture cap = init();
#endif
    
  char *buf = (char*)malloc(sizeof(char) * PACKET_SIZE);
  while (1) {
    
    int n = fread(buf, sizeof(char), PACKET_SIZE, stdin);
    int mode = get_mode(buf, n);
    int l = send(s, &mode, sizeof(int), 0);
    if (l != sizeof(int)) die("failes to send mode\n");
    int m = send(s, buf, mode, 0);
    if (mode != m) die("failed to send data\n");
#ifdef DEBUG
    fprintf(stderr, "send: mode=%d, size=%d\n", mode, m);
    fprintf(stderr, "finished sending sound data\n");
#endif
#ifdef USE_VIDEO
    cv::Mat img;
    get_image(cap, img);
    int img_size = sizeof(img);
    m = send(s, &img_size, sizeof(img_size), 0);
    if (m !=  sizeof(img_size)) die("failed to send img_size data\n");
    m = send(s, &img, sizeof(img), 0);
    if (m != sizeof(img)) die("failed to send img data\n");
#ifdef DEBUG
    fprintf(stderr, "finished sending img data\n");
#endif
#endif

    l = recv(s, &mode, sizeof(int), 0);
    n = recv(s, buf, sizeof(char) * mode, 0);
    fwrite(buf, sizeof(char), n, stdout);
#ifdef DEBUG
    fprintf(stderr, "recv: mode=%d, size=%d\n", mode, n);
    fprintf(stderr, "finished getting sound data\n");
#endif
#ifdef USE_VIDEO
    img_size = recv(s, &img_size, sizeof(img_size), 0);
    img = recv(s, &img, img_size, 0);
    print_image(img);
#ifdef DEBUG
    fprintf(stderr, "finished getting img data\n");
#endif

    if (check_key()) {
      break;
    }
#endif
  }
#ifdef USE_VIDEO
  cv::destroyAllWindows();
#endif USE_VIDEO
  close(s);
  return 0;
}
