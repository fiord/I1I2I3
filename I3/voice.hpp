#ifndef INCLUDE_GUARD_VOICE_HPP
#define INCLUDE_GUARD_VOICE_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#define PACKET_SIZE 1024
#define cut_volume 50

void zero_fill(char *buf) {
  for (int i=0; i<PACKET_SIZE; i++) {
    if (abs(buf[i]) < cut_volume) buf[i] = 0;
  }
}
#endif // INCLUDE_GUARD_VOICE_HPP
