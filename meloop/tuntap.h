#ifndef MELOOP_TUNTAP_H
#define MELOOP_TUNTAP_H


#include "meloop/io.h"

#include <stdbool.h>
#include <netinet/in.h>
#include <linux/if.h>


struct tunS {
    struct ioS;
    bool tap;
    int fd;
    char name[IFNAMSIZ];
    const char *address;
    const char *destaddress;
    const char *netmask;
    struct in_addr addressB;
    struct in_addr destaddressB;
    struct in_addr netmaskB;
};


void 
tunopenA(struct circuitS *c, void *s, void *data);


// void 
// tunreadA(struct circuitS *c, void *s, struct stringS *data);
// 
// 
// void
// tunencA(struct circuitS *c, void *s, struct stringS *data);


#endif
