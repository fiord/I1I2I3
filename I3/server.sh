#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone sound 12345 | play -t raw -b 16 -c 1 -e s -r 44100 -;
  fi

  if [ $1 = "video" ]; then
    gnome-terminal --command "./phone video 30000";
    rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone sound 12345 | play -t raw -b 16 -c 1 -e s -r 44100 -;
  fi
else
  echo "usage: ./server.sh (sound|video)"
fi
