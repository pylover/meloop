#include "meloop/addr.h"
#include "meloop/types.h"

#include <stdio.h>
#include <string.h>


static char addrtemp[256];


char *
meloop_addr_dump(struct sockaddr *addr) {
    struct sockaddr_in *addrin = (struct sockaddr_in*) addr;
    sprintf(addrtemp, "%s:%d", 
            inet_ntoa(addrin->sin_addr),
            ntohs(addrin->sin_port));
    return addrtemp;
}


int
meloop_addr_parse(struct sockaddr *addr, const char *host, 
        unsigned short port) {
    struct sockaddr_in *addrin = (struct sockaddr_in*)addr;

    memset(addrin, 0, sizeof(struct sockaddr_in));
    addrin->sin_family = AF_INET;
    if (host == NULL) {
        addrin->sin_addr.s_addr = htonl(INADDR_ANY);
    } 
    else if(inet_pton(AF_INET, host, &addrin->sin_addr) <= 0 ) {
        return ERR;
    }
    addrin->sin_port = htons(port); 
    return OK;
}
