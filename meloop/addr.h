#ifndef MELOOP_ADDR_H
#define MELOOP_ADDR_H


#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


int
addr_parse(struct sockaddr *addr, const char *host, unsigned short port);


char *
addr_dump(struct sockaddr *addr);


#endif
