#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

double x(double a, double f, double t) {
  return a*sin(2*M_PI*f*t);
}

int main(int argc, char **argv) {
  assert(argc == 3);
  double a = atof(argv[1]);
  int n = atoi(argv[2]);
  double base = 440 / pow(2, 10.0/12.0);
  int cnt = 0;

  for(int i = 0; i < n; i++) {
    int to = 13230;
    if((i%7) == 3 || (i%7) == 0) {
      base *= pow(2, 1.0/12.0);
    }
    else {
      base *= pow(2, 1.0/6.0);
    }
    for(int j = 0; j < to; j++) {
      double t = j / 44100.0;
      short val = x(a, base, t);
      fwrite(&val, 2, 1, stdout);
    }
  }
  return 0;
}
