#!/bin/sh
if [ $# -ge 1 ]; then
  if [ $1 = "sound" ]; then
    if [ $# -eq 2 ]; then
      ./phone $1 $2 12345;
    else
      echo "usage: ./client.sh sound {ip_addr}"
    fi
  fi

  if [ $1 = "video" ]; then
    if [ $# -eq 2 ]; then
      gnome-terminal --command "./phone video $2 30000";
      ./phone sound $2 12345;
    else
      echo "usage: ./client.sh video {ip_addr}"
    fi
  fi
else
  echo "usage: ./client.sh (sound|video) {ip_addr}"
fi
