#ifndef MELOOP_ADDR_H
#define MELOOP_ADDR_H


#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


int
meloop_sockaddr_parse(struct sockaddr *saddr, const char *addr, 
        unsigned short port);


char *
meloop_sockaddr_dump(struct sockaddr *addr);


#endif
