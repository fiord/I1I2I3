#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM

enum MODE {
  HOST,
  GUEST,
};

int main(int argc, char **argv) {
  assert(argc == 2 || argc == 3);
  int s;
  enum MODE mode;
  if (argc == 2) {
    // host
    mode = HOST;
    char **endptr = NULL;
    int port = strtol(argv[1], endptr, 0);
    assert(endptr == NULL);

    int ss = socket(PF_INET, TCP_STREAM, 0);
    assert(ss != -1);
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
    printf("connected.\n");
  }
  else if(argc == 3) {
    // guest
    mode = GUEST;
    char *to_addr = argv[1];
    char **endptr = NULL;
    int to_port= strtol(argv[2], endptr, 0);
    fprintf(stderr, "connecting to %s:%d...\n", to_addr, to_port);
    assert(endptr == NULL);
    s = socket(PF_INET, TCP_STREAM, 0);
    assert(s != -1);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    int ret = inet_aton(to_addr, &addr.sin_addr);
    assert(ret == 1);
    addr.sin_port = htons(to_port);
    ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
    int num = errno;
    if(ret == -1) {
      fprintf(stderr, "print error string by strerror: %s\n", strerror(num));
      fprintf(stderr, "print error code: %d\n", num);
    }
    assert(ret != -1);
    printf("connected.\n");
  }
  
  while (1) {
    // send
    short buf;
    int n = read(stdin, &buf, sizeof(short));
    if (n == 0) {
      // EOF
      break;
    }
    int m = send(s, &buf, sizeof(short), 0);

    // recv
    n = recv(s, &buf, sizeof(short), 0);
    if (n == 0) {
      // EOF
      break;
    }
    fwrite(&buf, sizeof(short), 1, stdout);
  }

  close(s);
  return 0;
}
