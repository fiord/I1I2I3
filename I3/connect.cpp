#include "connect.hpp"

int start_server(char *arg_port) {
  char **endptr = NULL;
  int port = strtol(arg_port, endptr, 0);
  if (endptr != NULL) die("port is not valid\n");

  int ss = socket(PF_INET, TCP_STREAM, 0);
  if (ss == 1)  die("socket error\n");
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  int ret = bind(ss, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1)  die("failed to bind\n");
  ret = listen(ss, 10);
  if (ret == -1)  die("failed to listen\n");

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int s = accept(ss, (struct sockaddr*)&client_addr, &len);

  close(ss);

  return s;
}

int connect_server(char *arg_ip, char *arg_port) {
  char *to_addr = arg_ip;
  char **endptr = NULL;
  int to_port = strtol(arg_port, endptr, 0);
  if (endptr != NULL) die("port is not valid\n");
  int s = socket(PF_INET, TCP_STREAM, 0);
  if (s == 1) die("socket error\n");
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  int ret = inet_aton(to_addr, &addr.sin_addr);
  if (ret ==-1) die("ip address is not valid\n");
  addr.sin_port = htons(to_port);
  ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1)  die("connect failed\n");
  fprintf(stderr, "connection success\n");

  return s;
}