#!/bin/sh
rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone $1 50000 | play -t raw -b 16 -c 1 -e s -r 44100 -
