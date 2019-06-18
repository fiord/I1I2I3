#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    ./phone sound 12345;
  fi

  if [ $1 = "video" ]; then
    gnome-terminal --command "./phone video 30000";
    ./phone sound 12345;
  fi
else
  echo "usage: ./server.sh (sound|video)"
fi
