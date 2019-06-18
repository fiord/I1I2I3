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

void hold(int s) {
  FILE *fp = fopen("sound.raw", "r");
  short *buf = (short *)malloc(PACKET_SIZE);
  int n = 0, m = 0;
  while(1) {
    n = fread(buf, sizeof(short), PACKET_SIZE, fp);
    if (n < PACKET_SIZE) {
      fseek(fp, 0, SEEK_SET);
    }
    m = send(s, buf, n*sizeof(short), 0);
    if (m != n*sizeof(short)) die("failed to send data in hold\n");
    n = recv(s, buf, sizeof(short)*PACKET_SIZE, 0);
  }
}

void send_recv_voice(int s) {
  short *buf = (short*)malloc(sizeof(short) * PACKET_SIZE);
  while (1) {
    
    int n = fread(buf, sizeof(short), PACKET_SIZE, stdin);
    zero_fill(buf);
    int m = send(s, buf, PACKET_SIZE*sizeof(short), 0);
    if (n*sizeof(short) != m) die("failed to send sound data");
#ifdef DEBUG
    fprintf(stderr, "finished sending sound data");
#endif

    n = recv(s, buf, sizeof(short) * PACKET_SIZE, 0);
    fwrite(buf, 1, n, stdout);
#ifdef DEBUG
    fprintf(stderr, "finished getting sound data");
#endif
  }
  close(s);
}
      
