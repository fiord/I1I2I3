#!/bin/sh

if [ $1 = "sound" ]; then
rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone $1 $2 12345 | play -t raw -b 16 -c 1 -e s -r 44100 -;
fi

if [ $1 = "video" ]; then
rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone $1 $2 54321 32154 | play -t raw -b 16 -c 1 -e s -r 44100 - &
./phone $1 $2 54321 32154
fi
