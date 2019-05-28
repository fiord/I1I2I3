#!/bin/sh
rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone 192.168.100.17 50000 | play -t raw -b 16 -c 1 -e s -r 44100 -
