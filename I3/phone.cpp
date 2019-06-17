#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <limits.h>
#include <thread>
#include "die.hpp"
#include "connect.hpp"
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM
#define PACKET_SIZE 1024

#undef DEBUG

#include "voice.hpp"
#include "video.hpp"

int main(int argc, char **argv) {
  int s;
  std::thread video_thread;

  if (argc >= 2) {
    if (strcmp(argv[1], "sound") == 0)  {
      if (argc == 3) {
        s = start_server(argv[2]);
      }
      else if(argc == 4) {
        s = connect_server(argv[2], argv[3]);
      }
      else {
        die("wrong usage: ./phone sound [port] or ./phone sound [ip] [port]\n");
      }
    }
    else if (strcmp(argv[1], "video")) {
      if (argc == 4) {
        s = start_server(argv[2]);
        video_thread = std::thread(video_server, argv[3]);
      }
      else if (argc == 5) {
        s = connect_server(argv[2], argv[3]);
        video_thread = std::thread(video_client, argv[2], argv[4]);
      }
      else {
        die("wrong usage: ./phone video [port] [port] or ./phone video [ip] [port] [port]\n");
      }
    }
  }
  else {
    die("wrong usage: ./phone (sound|video)\n");
  }

  short *buf = (short*)malloc(sizeof(short) * PACKET_SIZE);
  while (1) {
    
    int n = fread(buf, sizeof(short), PACKET_SIZE, stdin);
    // zero_fill(buf);
    int m = send(s, buf, PACKET_SIZE, 0);
    if (n!= m) die("failed to send sound data\n");
#ifdef DEBUG
    fprintf(stderr, "send: mode=%d, size=%d\n", mode, m);
    fprintf(stderr, "finished sending sound data\n");
#endif

    n = recv(s, buf, sizeof(short) * PACKET_SIZE, 0);
    fwrite(buf, sizeof(short), n, stdout);
#ifdef DEBUG
    fprintf(stderr, "recv: mode=%d, size=%d\n", mode, n);
    fprintf(stderr, "finished getting sound data\n");
#endif
  }
  close(s);
  return 0;
}
