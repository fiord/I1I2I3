#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int read(int fd, void* buf, int num);

int main(int argc, char **argv) {
  assert(argc == 2);
  int fd = open(argv[1], O_RDONLY);
  int cnt = 0;
  while (1) {
    short buf = 0;
    int n = read(fd, &buf, 2);
    if(n == -1) {
      perror("read");
      exit(1);
    }
    if(n == 0)  break;
    printf("%d %d\n", cnt++, buf);
  }
  return 0;
}
