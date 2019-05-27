/* 
 * p06.c
 */

/* 
 * usage:
 * 
 *   ./a.out
 *
 * Intented behavior:
 * It should print the content of this file.
 *
 */
#include <fcntl.h>

int read(int fd, void *buf, int num);
int write(int fd, void *buf, int num);

int main()
{
  int fd = open("p06.c", O_RDONLY);
  char buf[100];
  while (1) {
    int n = read(fd, buf, 100);
    if (n == 0) break;
    write(1, (void*)buf, n);
  }
  return 0;
}
