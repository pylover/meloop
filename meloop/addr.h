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


int
meloop_in_addr_parse(const char *addr, struct in_addr *inaddr);


#endif
