#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    ./phone sound 12345 2> std-err.txt;
  fi

  if [ $1 = "video" ]; then
    gnome-terminal --command "./phone sound 12345";
    gnome-terminal --command "./Build/vrphone.x86_64";
    ./phone video 54321 2> std-err.txt;
  fi
else
  echo "usage: ./server.sh (sound|video)"
fi
