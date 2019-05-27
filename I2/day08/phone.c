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

int main(int argc, char **argv) {
  assert(argc == 2 || argc == 3);
  int s;
  if (argc == 2) {
    // host
    char **endptr = NULL;
    int port = strtol(argv[1], endptr, 0);
    assert(endptr == NULL);

    int ss = socket(PF_INET, TCP_STREAM, 0);
    assert(ss != 1);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(ss, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(ss, 10);
    assert(ret != -1);

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    s = accept(ss, (struct sockaddr*)&client_addr, &len);

    close(ss);
  }
  else if(argc == 3) {
    // guest
    char *to_addr = argv[1];
    char **endptr = NULL;
    int to_port= strtol(argv[2], endptr, 0);
    assert(endptr == NULL);
    s = socket(PF_INET, TCP_STREAM, 0);
    assert(s != 1);
    struct sockaddr_in addr;
    int ret = inet_aton(to_addr, &addr.sin_addr);
    assert(ret != -1);
    addr.sin_port = htons(to_port);
    ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret != -1);
  }
  
  FILE *rec = popen("rec -t raw -c 1 -r 44100 -", "r");

  while (1) {
    // send
    char buf;
    int n = fread(&buf, sizeof(char), 1, rec);
    if (n == 0) {
      // EOF
      break;
    }
    int m = send(s, &buf, sizeof(char), 0);
    assert(n == m);

    // recv
    n = recv(s, &buf, sizeof(char), 0);
    if (n == 0) {
      // EOF
      break;
    }
    fwrite(&buf, sizeof(char), 1, stdout);
  }
  pclose(rec);
  close(s);
  return 0;
}
