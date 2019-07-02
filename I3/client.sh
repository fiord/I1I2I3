#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    if [ $# -eq 2 ]; then
      ./phone $1 $2 12345 2> std-err.txt;
    else
      echo "usage: ./client.sh sound {ip_addr}"
    fi
  fi

  if [ $1 = "video" ]; then
    if [ $# -eq 2 ]; then
      gnome-terminal --command "./phone sound $2 12345";
      gnome-terminal --command "./Build/vrphone.x86_64";
      ./phone video $2 54321 2> std-err.txt;
    else
      echo "usage: ./client.sh video {ip_addr}"
    fi
  fi
else
  echo "usage: ./client.sh (sound|video) {ip_addr}"
fi
