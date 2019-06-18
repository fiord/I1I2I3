#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    if [ $# -eq 2 ]; then
      rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone $1 $2 12345 | play -t raw -b 16 -c 1 -e s -r 44100 -;
    else
      echo "usage: ./client.sh sound {ip_addr}"
    fi
  fi

  if [ $1 = "video" ]; then
    if [ $# -eq 2 ]; then
      gnome-terminal --command "./phone video $2 30000";
      rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone sound $2 12345 | play -t raw -b 16 -c 1 -e s -r 44100 -;
    else
      echo "usage: ./client.sh video {ip_addr}"
    fi
  fi
else
  echo "usage: ./client.sh (sound|video) {ip_addr}"
fi
