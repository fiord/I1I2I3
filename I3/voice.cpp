#include "voice.hpp"

#define PACKET_SIZE 10240
#define cut_volume 30

void zero_fill(short *buf) {
  for (int i=0; i<PACKET_SIZE; i++) {
    if (abs(buf[i]) > cut_volume) return;
  }
  for (int i=0; i<PACKET_SIZE; i++) {
    buf[i] = 0;
  }
}
