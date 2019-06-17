#!/bin/sh
if [ $1 = "sound" ]; then
rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone sound 12345 | play -t raw -b 16 -c 1 -e s -r 44100 -;
fi

if [ $1 = "video" ]; then
  (
  trap "kill 0" EXIT
    rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone sound 12345 | play -t raw -b 16 -c 1 -e s -r 44100 - &
    ./phone video 30000 &
  wait
  )
fi
