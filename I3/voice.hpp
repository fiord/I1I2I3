#ifndef INCLUDE_GUARD_VOICE_HPP
#define INCLUDE_GUARD_VOICE_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#define cut_volume 50

int get_mode(char *buf, int n) {
  for (int i=0; i<n; i++) {
    if (abs(buf[i]) < cut_volume) return n;
  }
  return 0;
}
#endif // INCLUDE_GUARD_VOICE_HPP
