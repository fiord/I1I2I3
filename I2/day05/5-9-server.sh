#!/bin/sh
while true; do
  (cat $1) | nc -l -u 12345;
  [ $? != 0 ] && break;
done
