#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

double x(double a, double f, double t) {
  return a*sin(2*M_PI*f*t);
}

int main(int argc, char **argv) {
  assert(argc == 4);
  double a = atof(argv[1]);
  double f = atof(argv[2]);
  int n = atoi(argv[3]);

  for(int i = 0; i < n; i++) {
    double t = i / 44100.0;
    short val = x(a, f, t);
    fwrite(&val, 2, 1, stdout);
  }
  return 0;
}
