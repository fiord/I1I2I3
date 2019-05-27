#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  assert(argc == 2);
  int n = atoi(argv[1]);
  while(1) {
    short *buf = (short*)malloc(sizeof(short) * n);
    int m = fread(buf, 2, n, stdin);
    if(m == 0)  break;
    fwrite(buf, 2, 1, stdout);
    if(m != n)  break;
  }
  return 0;
}
