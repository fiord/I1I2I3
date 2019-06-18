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
#include "log.hpp"
#include "connect.hpp"
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM
#define PACKET_SIZE 10240

#undef DEBUG

#include "voice.hpp"
#include "video.hpp"

int main(int argc, char **argv) {
  try {
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
          die("wrong usage: ./phone sound [port] or ./phone sound [ip] [port]");
        }
      }
      else if (strcmp(argv[1], "video") == 0) {
        fprintf(stderr, "[info] This is video mode.\n");
        if (argc == 3) {
          fprintf(stderr, "start server at port %s...\n", argv[2]);
          int t = start_server(argv[2]);
          send_recv(t);
        }
        else if (argc == 4) {
          fprintf(stderr, "connect to %s:%s...\n", argv[2], argv[3]);
          int t = connect_server(argv[2], argv[3]);
          send_recv(t);
        }
        else {
          die("wrong usage: ./phone video [port] [port] or ./phone video [ip] [port] [port]");
        }
      }
      else {
        die("wrong usage: ./phone (sound|video)");
      }
    }
    else {
      die("wrong usage: ./phone (sound|video)");
    }


    if(strcmp(argv[1], "video")) {
      send_recv_voice(s);
    }
  }
  catch (char *str) {
    if (strcmp(argv[1], "video") == 0) {
      fprintf(stderr, "[error] %s\n", str);
      fprintf(stderr, "press enter to exit:");
      char c;
      scanf("%c", c);
    }
  }
  return 0;
}
