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
  exit(1);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  char **endptr = NULL;
  int port = strtol(argv[1], endptr, 0);
  if (endptr != NULL) die("port is not valid");

  int ss = socket(PF_INET, TCP_STREAM, 0);
  if (ss == 1)  die("socket error");
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  int ret = bind(ss, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1)  die("failed to bind");
  ret = listen(ss, 10);
  if (ret == -1)  die("failed to listen");

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int s = accept(ss, (struct sockaddr*)&client_addr, &len);

  close(ss);

  while (1) {
    char buf;
    int n = fread(&buf, sizeof(char), 1, stdin);
    if (n == 0) {
      // EOF
      break;
    }
    int m = send(s, &buf, sizeof(char), 0);
    if (n != m) die("failed to send data");
  }
  close(s);
  return 0;
}
