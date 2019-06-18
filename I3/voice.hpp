#ifndef INCLUDE_GUARD_VOICE_HPP
#define INCLUDE_GUARD_VOICE_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "log.hpp"

extern void zero_fill(short *);

extern void hold(int);

extern void send_recv_voice(int);

#endif // INCLUDE_GUARD_VOICE_HPP
