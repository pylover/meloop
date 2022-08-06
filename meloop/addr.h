#ifndef MELOOP_ADDR_H
#define MELOOP_ADDR_H


#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


int
meloop_addr_parse(struct sockaddr *addr, const char *host, 
        unsigned short port);


char *
meloop_addr_dump(struct sockaddr *addr);


#endif
