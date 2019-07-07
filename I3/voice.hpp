#ifndef INCLUDE_GUARD_VOICE_HPP
#define INCLUDE_GUARD_VOICE_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <complex>
#include <mutex>
#include "log.hpp"

extern void send_recv_voice(int);

#endif // INCLUDE_GUARD_VOICE_HPP
