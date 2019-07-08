#include <ncurses.h>
#include "voice.hpp"

#define PACKET_SIZE 1025
#define cut_volume 500
#define smallest_fre 100
#define largest_fre 5000

char my_command = 0;
char your_command = 0;

std::mutex mutex;

void write_cursor(WINDOW *for_read, int y_now) {
  mutex.lock();
  wclear(for_read);
  wmove(for_read, y_now, 0);
  wechochar(for_read, '>');
  wrefresh(for_read);
  mutex.unlock();
}

void get_command(WINDOW *for_read) {
  sleep(0.5);

  char buf = 0;
  int y_now = 0;
  char my_com_buf = 0, your_com_buf = 0;

  while(1) {
    if (my_command == 'q' || your_command == 'q') return;
    while ((buf = wgetch(for_read)) != '\n') {
      if (buf == ' ' && your_command != 'h') {
        y_now = 1 - y_now;
        write_cursor(for_read, y_now);
      }
    }
    my_com_buf = my_command, your_com_buf = your_command;
    if (y_now == 0) {
      if (my_com_buf == 'h') my_command = 'f';
      else if (your_com_buf == 'h') my_command = 'q';
      else my_command = 'h';
    }
    else if (y_now == 1) {
      if (my_com_buf == 'h') my_command = 'q';
      else if (your_com_buf != 'h') my_command = 'q';
    }
    y_now = 0;
    write_cursor(for_read, y_now);
  }
}

void display() {
  std::chrono::system_clock::time_point start, now;
  int time = 0, pre_time = 0;
  start = std::chrono::system_clock::now();

  initscr();
  cbreak();
  noecho();
  curs_set(0);

  WINDOW *for_write = newwin(5, 100, 0, 0);
  WINDOW *for_read = newwin(2, 1, 3, 0);
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
      if (my_com_buf == 'h') waddstr(for_write, "holding\n\n\n  back to talk\n  hang up");
      else if (your_com_buf == 'h') waddstr(for_write,"on hold\n\n\n  hang up");
      else waddstr(for_write, "talking\n\n\n  hold\n  hang up");
      wmove(for_write, 1, 0);
      wprintw(for_write, "time : %02d:%02d", time/60, time%60);
      wrefresh(for_write);
      mutex.unlock();
      write_cursor(for_read, 0);
    }
    now = std::chrono::system_clock::now();
    pre_time = time;
    time = (int)std::chrono::duration_cast<std::chrono::seconds>(now-start).count();
    if (pre_time != time) {
      mutex.lock();
      wmove(for_write, 1, 0);
      wprintw(for_write, "time : %02d:%02d", time/60, time%60);
      wrefresh(for_write);
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

void fft_r(std::complex<double> *x, std::complex<double> *y, int n, std::complex<double> w) {
  if (n == 1) y[0] = x[0];
  else {
    std::complex<double> W = 1.0;
    for (int i=0; i<n/2; i++) {
      y[i] = x[i] + x[i+n/2];
      y[i+n/2] = W * (x[i] - x[i+n/2]);
      W *= w;
    }
    fft_r(y, x, n/2, w*w);
    fft_r(y+n/2, x+n/2, n/2, w*w);
    for (int i=0; i<n/2; i++) {
      y[2*i] = x[i];
      y[2*i+1] = x[i+n/2];
    }
  }
}

void bandpass(short *buf, std::complex<double> *x, std::complex<double> *y) {
  int n = PACKET_SIZE-1;
  for (int i=0; i<n; i++) {
    x[i] = buf[i+1];
  }
  double arg = 2.0 * M_PI / n;
  std::complex<double> w(cos(arg), -sin(arg));
  fft_r(x, y, n, w);
  for (int i=0; i<n; i++) {
    y[i] /= n;
  }
  for (int i=0; i<=n/2; i++) {
    double buf = i * 44100 / (double)n;
    if (buf <= smallest_fre || buf >= largest_fre) {
      if (i == 0) y[0] = 0.0;
      else if (i == n/2) y[i] == 0.0;
      else {
        y[i] = 0.0;
        y[n-i] = y[i];
      }
    }
  }
  w = std::complex<double> (cos(arg), sin(arg));
  fft_r(y, x, n, w);
  for (int i=0; i<n; i++) {
    buf[i+1] = x[i].real();
  }
}
 
void zero_fill(short *buf) {
  for (int i=0; i<PACKET_SIZE-1; i++) {
    if (abs(buf[i]) > cut_volume) return;
  }
  for (int i=0; i<PACKET_SIZE-1; i++) {
    buf[i] = (short)0;
  }
}

void send_voice(int s) {
  FILE *sound_in = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
  if (sound_in == NULL) die("failed to invoke rec\n");
  short *buf = (short *)malloc(sizeof(short) * PACKET_SIZE);
  std::complex<double> *x = (std::complex<double> *)calloc(sizeof(std::complex<double>), PACKET_SIZE-1);
  std::complex<double> *y = (std::complex<double> *)calloc(sizeof(std::complex<double>), PACKET_SIZE-1);

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
      sound_in = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    }
    else if (your_com_buf == 'h') {
      pclose(sound_in);
      recv(s, buf, PACKET_SIZE*sizeof(short), 0);
      your_command = (char)(buf[0]);
      sound_in = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    }
    else {
      buf[0] = 0;
      int n = fread(&buf[1], sizeof(short), PACKET_SIZE-1, sound_in);
      // bandpass(buf, x, y);
      zero_fill(buf+1);
      int m = send(s, buf, (n+1)*sizeof(short), 0);
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
  FILE *sound_out = popen("play -t raw -b 16 -c 1 -e s -r 44100 -", "w");
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
      sound_out = popen("play -t raw -b 16 -c 1 -e s -r 44100 -", "w");
    }
    else if (your_com_buf == 'h') {
      FILE *music = fopen("sound/for_horyu.raw", "r");
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
        fwrite(&buf[1], 1, n-sizeof(short), sound_out);
      }
    }
    
#ifdef DEBUG
    fprintf(stderr, "finished getting sound data");
#endif
  }

  pclose(sound_out);
  system("play -q -t raw -b 16 -c 1 -e s -r 44100 sound/end.raw");
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
