#include <ncurses.h>
#include "voice.hpp"

#define PACKET_SIZE 1024
#define cut_volume 30

char command = 0;
 
void get_command() {
  initscr();
  nocbreak();
  echo();
  timeout(100);
  sleep(0.5);

  char buf = 0;
  while(1) {
    if (command == 'q') break;
    if (command == 'z') {
      clear();
      move(0, 0);
      addstr("horyu-chu\n");
      refresh();
      while (command == 'z') {}
    }
    clear();
    move(0, 0);
    if (command == 'h') addstr("put f and press enter to finish hold\n");
    else addstr("put command and press enter\n hold : h\n finish talking : q\n");
    refresh();
    char pre_com = command;
    int x_posi = 0, y_posi = 0;
    while ((buf = getch()) < 0) {
      move(3, 0);
      refresh();
      if (command != pre_com) break;
    }
    if (buf > 0) {
      while (getch() != '\n') {}
    }
    if (buf == 'h' || buf == 'f' || buf == 'q') command = buf;
  }

  endwin();
}

void zero_fill(short *buf) {
  for (int i=0; i<PACKET_SIZE-1; i++) {
    if (abs(buf[i]) > cut_volume) return;
  }
  for (int i=0; i<PACKET_SIZE-1; i++) {
    buf[i] = 0;
  }
}

void hold(int s) {
  int fd = open("for_horyu.raw", O_RDONLY);
  if (fd == -1) die("open sound in hold\n");
  short *buf = (short *)malloc(sizeof(short) * PACKET_SIZE);
  int n = 0, m = 0;
  while (command != 'f') {
    buf[0] = 'z';
    n = read(fd, buf+1, PACKET_SIZE-1);
    if (n == -1) die("failed to read in hold\n");
    if (n < PACKET_SIZE) {
      lseek(fd, 0, SEEK_SET);
    }
    m = send(s, buf, PACKET_SIZE*sizeof(short), 0);
    if ((n+1)*sizeof(short) != m) die("failed to send in hold\n");
  }
  close(fd);
}

void send_voice(int s) {
  FILE *sound_in = popen("rec -q -t raw -b 16 -c 1 -e s -r 44100 -", "r");
  if (sound_in == NULL) die("failed to invoke rec\n");
  short *buf = (short*)malloc(sizeof(short) * PACKET_SIZE);
  while (1) {

    if (command == 'q') {
      buf[0] = 'q';
      send(s, buf, PACKET_SIZE*sizeof(short), 0);
      break;
    }
    else if (command == 'h') {
      hold(s);
    }
    else if  (command == 'z') {
      while (command == 'z') {}
    }
    else {
      buf[0] = 0;
      int n = fread(buf+1, sizeof(short), PACKET_SIZE-1, sound_in);
      zero_fill(buf+1);
      int m = send(s, buf, PACKET_SIZE*sizeof(short), 0);
      if ((n+1)*sizeof(short) != m) die("failed to send sound data");
    }

#ifdef DEBUG
    fprintf(stderr, "finished sending sound data");
#endif
  }
  pclose(sound_in);
  free(buf);
}

void recv_voice(int s) {
  FILE *sound_out = popen("play -q -t raw -b 16 -c 1 -e s -r 44100 -", "w");
  if (sound_out == NULL) die("failed to invoke play\n");
  short *buf = (short *)malloc(sizeof(short) * PACKET_SIZE);
  while (1) {

    if (command == 'q') break;
    else if (command == 'h') {
      while (command == 'h') {}
    }
    else {
      int n = recv(s, buf, sizeof(short) * PACKET_SIZE, 0);
      if (buf[0] == 'q') break;
      else if (buf[0] == 'z') command = 'z';
      if (command == 'z' && buf[0] != 'z') command = 0;
      fwrite(buf+1, 1, n-sizeof(short), sound_out);
    }
    
#ifdef DEBUG
    fprintf(stderr, "finished getting sound data");
#endif
  }
  pclose(sound_out);
  free(buf);
}

void send_recv_voice(int s) {
  std::thread sender(send_voice, s);
  std::thread recver(recv_voice, s);
  std::thread commander(get_command);

  sender.join();
  recver.join();
  commander.join();
  
  close(s);
}
