#include <ncurses.h>
#include "voice.hpp"

#define PACKET_SIZE 1024
#define cut_volume 30

char my_command = 0;
char your_command = 0;

std::mutex mutex;

void get_command(WINDOW *for_read) {
  sleep(0.5);

  char buf = 0;
  while(1) {
    if (my_command == 'q' || your_command == 'q') return;
    buf = wgetch(for_read);
    while (wgetch(for_read) != '\n') {}
    if (buf == 'h' || buf == 'f') {
      if (your_command != 'h') {
        my_command = buf;
      }
    }
    else if (buf == 'q') {
      my_command = 'q';
    }
    mutex.lock();
    wclear(for_read);
    wmove(for_read, 0, 0);
    wrefresh(for_read);
    mutex.unlock();
  }
}

void display() {
  std::chrono::system_clock::time_point start, now;
  int time = -1, pre_time = -1;
  start = std::chrono::system_clock::now();

  initscr();
  nocbreak();
  echo();
  curs_set(1);

  WINDOW *for_write = newwin(5, 100, 0, 0);
  WINDOW *for_read = newwin(5, 100, 5, 0);
  sleep(0.5);

  std::thread get_commander(get_command, for_read);

  char pre_my_com = -1, pre_your_com = -1;
  char my_com_buf = -1, your_com_buf = -1;
  int x_now = 0, y_now = 0;
  while (1) {
    my_com_buf = my_command, your_com_buf = your_command;
    if (my_com_buf != pre_my_com || your_com_buf != pre_your_com) {
      if (my_com_buf == 'q' || your_com_buf == 'q') break;
      mutex.lock();
      wclear(for_write);
      wmove(for_write, 0, 0);
      if (your_com_buf == 'h') waddstr(for_write, "horyu-chu\n");
      else if (my_com_buf == 'h') waddstr(for_write, "put f and press enter to finish hold\n");
      else waddstr(for_write, "put command and press enter\n hold : h\n finish talking : q\n");
      wrefresh(for_write);
      mutex.unlock();
    }
    now = std::chrono::system_clock::now();
    pre_time = time;
    time = (int)std::chrono::duration_cast<std::chrono::seconds>(now-start).count();
    if (pre_time != time) {
      mutex.lock();
      getyx(for_write, y_now, x_now);
      wmove(for_write, y_now, 0);
      wprintw(for_write, "time : %02d:%02d", time/60, time%60);
      wrefresh(for_write);
      getyx(for_read, y_now, x_now);
      wmove(for_read, y_now, x_now);
      wrefresh(for_read);
      mutex.unlock();
    }
    pre_my_com = my_com_buf;
    pre_your_com = your_com_buf;
    sleep(0.05);
  }

  get_commander.detach();
  wclear(for_read);
  wclear(for_write);
  wrefresh(for_read);
  wrefresh(for_write);
  delwin(for_read);
  delwin(for_write);
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

void send_voice(int s) {
  FILE *sound_in = popen("rec -q -t raw -b 16 -c 1 -e s -r 44100 -", "r");
  if (sound_in == NULL) die("failed to invoke rec\n");
  short *buf = (short*)malloc(sizeof(short) * PACKET_SIZE);

  char my_com_buf = -1, your_com_buf = -1;
  while (1) {
    my_com_buf = my_command, your_com_buf = your_command;

    if (my_com_buf == 'q') {
      buf[0] = 'q';
      send(s, buf, PACKET_SIZE*sizeof(short), 0);
      break;
    }
    else if (your_com_buf == 'q') {
      break;
    }
    else if (my_com_buf == 'h') {
      pclose(sound_in);
      buf[0] = 'h';
      send(s, buf, PACKET_SIZE*sizeof(short), 0);
      while (1) {
        my_com_buf = my_command;

        if (my_com_buf == 'f') {
          buf[0] = 'f';
          break;
        }
        else if (my_com_buf == 'q') {
          buf[0] = 'q';
          break;
        }

        sleep(0.05);
      }
      send(s, buf, PACKET_SIZE*sizeof(short), 0);
      sound_in = popen("rec -q -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    }
    else if (your_com_buf == 'h') {
      pclose(sound_in);
      recv(s, buf, PACKET_SIZE*sizeof(short), 0);
      your_command = (char)(buf[0]);
      sound_in = popen("rec -q -t raw -b 16 -c 1 -e s -r 44100 -", "r");
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

  char my_com_buf = -1, your_com_buf = -1;
  while (1) {
    my_com_buf = my_command, your_com_buf = your_command;

    if (my_com_buf == 'q' || your_com_buf == 'q') break;
    else if (my_com_buf == 'h') {
      pclose(sound_out);
      while (1) {
        my_com_buf = my_command;
        if (my_com_buf == 'f' || my_com_buf == 'q') break;

        sleep(0.05);
      }
      sound_out = popen("play -q -t raw -b 16 -c 1 -e s -r 44100 -", "w");
    }
    else if (your_com_buf == 'h') {
      pclose(sound_out);
      sound_out = popen("play --buffer 256 -q -t raw -b 16 -c 1 -e s -r 44100 -", "w");
      FILE *music = fopen("for_horyu.raw", "r");
      if (music == NULL) die("failed to open music\n");
      while (1) {
        my_com_buf = my_command, your_com_buf = your_command;

        int n = fread(buf, sizeof(short), PACKET_SIZE, music);
        if (n < PACKET_SIZE) {
          fseek(music, 0, SEEK_SET);
        }
        int m = fwrite(buf, 1, n*sizeof(short), sound_out);
        if (n*sizeof(short) != m) die("failed to play music\n");
        if (your_com_buf == 'f' || your_com_buf == 'q' || my_com_buf == 'q') {
          break;
        }
      }
      fclose(music);
      pclose(sound_out);
      sound_out = popen("play -q -t raw -b 16 -c 1 -e s -r 44100 -", "w");
    }
    else {
      int n = recv(s, buf, sizeof(short) * PACKET_SIZE, 0);
      if (buf[0] == 'q') {
        your_command = 'q';
        break;
      }
      else if (buf[0] == 'h') {
        your_command = 'h';
      }
      else {
        fwrite(buf+1, 1, n-sizeof(short), sound_out);
      }
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
  std::thread commander(display);

  sender.join();
  recver.join();
  commander.join();
  
  close(s);
}
