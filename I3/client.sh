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
    if [ $# -eq 3 ]; then
      gnome-terminal --command "./Build/vrphone.x86_64";
      sleep 5;
      gnome-terminal --command "./phone video $2 $3 54321";
      ./phone sound $3 12345 2> std-err.txt
    else
      echo "usage: ./client.sh video {characterID} {ip_addr}"
    fi
  fi
else
  echo "usage: ./client.sh (sound|video) {ip_addr}"
fi
