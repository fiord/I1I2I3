#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    ./phone sound 12345 2> std-err.txt;
  fi

  if [ $1 = "video" ]; then
    if [ $# -eq 2 ]; then
      gnome-terminal --command "./Build/vrphone.x86_64";
      sleep 5
      gnome-terminal --command "./phone video $2 54321";
      ./phone sound 12345 2> std-err.txt
    else
      echo "usage: ./server.sh video (characterID)"
    fi
  fi
else
  echo "usage: ./server.sh (sound|video)"
fi
