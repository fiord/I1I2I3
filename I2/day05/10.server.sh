#!/bin/sh
rec -c 1 -r 44100 -p | nc -l -u 12345
