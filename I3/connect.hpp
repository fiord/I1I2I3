#ifndef INCLUDE_GUARD_CONNECT_HPP
#define INCLUDE_GUARD_CONNECT_HPP
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "die.hpp"
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM
#define SOUND_PACKET 1024

extern int start_server(char *arg_port);

extern int connect_server(char *arg_ip, char *arg_port);

#endif // INCLUDE_GUARD_CONNECT_HPP
