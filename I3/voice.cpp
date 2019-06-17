#include "voice.hpp"

#define PACKET_SIZE 1024
#define cut_volume 30

void zero_fill(short *buf) {
  for (int i=0; i<PACKET_SIZE; i++) {
    if (abs(buf[i]) < cut_volume) buf[i] = 0;
  }
}
