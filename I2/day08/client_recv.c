#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM

void die(char *s) {
  perror(s);
  exit(-1);
}

int main(int argc, char **argv) {
  assert(argc == 3);

  char *to_addr = argv[1];
  char ** endptr = NULL;
  int to_port = strtol(argv[2], endptr, 0);
  if (endptr != NULL) die("port is not valid");
  int s = socket(PF_INET, TCP_STREAM, 0);
  if (s == 1)  die("socket error");
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  int ret = inet_aton(to_addr, &addr.sin_addr);
  if (ret == -1)  die("ip address is not valid");
  addr.sin_port = htons(to_port);
  ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1) die("connect failed");

  while(1) {
    char buf;
    int n = recv(s, &buf, sizeof(char), 0);
    if(n == 0) {
      // EOF
      break;
    }
    fwrite(&buf, sizeof(char), 1, stdout);
  }

  close(s);

  return 0;
}
