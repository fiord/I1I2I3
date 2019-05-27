#!/bin/sh
rec -t raw -b 16 -c 1 -e s -r 44100 | ./serv_send2 $2 & \
  ./client_recv $1 $2 | play -t raw -b 16 -c 1 -e s -r 44100 -
