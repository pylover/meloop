#include "meloop/addr.h"
#include "meloop/arrow.h"

#include <stdio.h>
#include <string.h>


static char addrtemp[256];


char *
meloop_sockaddr_dump(struct sockaddr *addr) {
    struct sockaddr_in *addrin = (struct sockaddr_in*) addr;
    sprintf(addrtemp, "%s:%d", 
            inet_ntoa(addrin->sin_addr),
            ntohs(addrin->sin_port));
    return addrtemp;
}


int
meloop_sockaddr_parse(struct sockaddr *saddr, const char *addr, 
        unsigned short port) {
    struct sockaddr_in *addrin = (struct sockaddr_in*)saddr;

    memset(addrin, 0, sizeof(struct sockaddr_in));
    addrin->sin_family = AF_INET;
    if (addr == NULL) {
        addrin->sin_addr.s_addr = htonl(INADDR_ANY);
    } 
    else if(inet_pton(AF_INET, addr, &addrin->sin_addr) <= 0 ) {
        return ERR;
    }
    addrin->sin_port = htons(port); 
    return OK;
}


int
meloop_in_addr_parse(const char *saddr, struct in_addr *inaddr) {
    struct sockaddr_in tmp;
    if (meloop_sockaddr_parse((struct sockaddr*) &tmp, saddr, 0)) {
        return ERR;
    }
    memcpy(inaddr, &tmp.sin_addr, sizeof(struct in_addr));
    return OK;
}
