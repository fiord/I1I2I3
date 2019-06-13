#ifndef INCLUDE_GUARD_IMG_HPP
#define INCLUDE_GUARD_IMG_HPP
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <cstdlib>
#include "die.hpp"

static void send_recv(int);

extern void video_server(char*);

extern void video_client(char*, char*);

#endif // INCLUDE_GUARD_IMG_HPP