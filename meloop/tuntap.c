#include "meloop/arrow.h"
#include "meloop/tuntap.h"
#include "meloop/io.h"
#include "meloop/addr.h"
#include "meloop/logging.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>

// TODO: delete it
// #if __UAPI_DEF_IF_IFREQ
// struct ifreq {
// #define IFHWADDRLEN	6
// 	union
// 	{
// 		char	ifrn_name[IFNAMSIZ];		/* if name, e.g. "en0" */
// 	} ifr_ifrn;
// 
// 	union {
// 		struct	sockaddr ifru_addr;
// 		struct	sockaddr ifru_dstaddr;
// 		struct	sockaddr ifru_broadaddr;
// 		struct	sockaddr ifru_netmask;
// 		struct  sockaddr ifru_hwaddr;
// 		short	ifru_flags;
// 		int	ifru_ivalue;
// 		int	ifru_mtu;
// 		struct  ifmap ifru_map;
// 		char	ifru_slave[IFNAMSIZ];	/* Just fits the size */
// 		char	ifru_newname[IFNAMSIZ];
// 		void __user *	ifru_data;
// 		struct	if_settings ifru_settings;
// 	} ifr_ifru;
// };
// #endif /* __UAPI_DEF_IF_IFREQ */


static int
_configure_interface(struct tunP *tun) {
    struct ifreq ifr;
    struct sockaddr_in *tmp = (struct sockaddr_in*) &ifr.ifr_addr;
    memset(&ifr, 0, sizeof(struct ifreq));

    /* Interface name */
    strncpy(ifr.ifr_name, tun->name, IFNAMSIZ-1);

    /* Address */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return ERR;
    }

    /* addr */
    if (!inet_pton(AF_INET, tun->address, &(tmp->sin_addr))) {
        return ERR;
    }
     
    tmp->sin_family = AF_INET;
    tmp->sin_port = 0;
    tun->addressB = tmp->sin_addr;
    
    if (ioctl(fd, SIOCSIFADDR, &ifr) == -1) {
        ERROR("errored: %s", inet_ntoa(tun->addressB));
        return ERR;
    }

    /* Destination Address */
    tmp = (struct sockaddr_in*) &ifr.ifr_dstaddr;
    if (!inet_pton(AF_INET, tun->destaddress, &(tmp->sin_addr))) {
        return ERR;
    }
     
    tmp->sin_family = AF_INET;
    tmp->sin_port = 0;
    tun->destaddressB = tmp->sin_addr;
    DEBUG("dest: %s", inet_ntoa(tun->destaddressB));
    
    if (ioctl(fd, SIOCSIFDSTADDR, &ifr) == -1) {
        ERROR("errored: %s", inet_ntoa(tun->destaddressB));
        return ERR;
    }

    /* Netmask */
    tmp = (struct sockaddr_in*) &ifr.ifr_netmask;
    if (!inet_pton(AF_INET, tun->netmask, &(tmp->sin_addr))) {
        return ERR;
    }
     
    tmp->sin_family = AF_INET;
    tmp->sin_port = 0;
    tun->netmaskB = tmp->sin_addr;

    /* Interface name */
    strncpy(ifr.ifr_name, tun->name, IFNAMSIZ-1);
    
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) == -1) {
        ERROR("errored: %s", inet_ntoa(tun->netmaskB));
        return ERR;
    }

    return OK;
}


void 
tunopenA(struct circuitS *c, void *s, void *data, struct tunP *priv) {
    struct ifreq ifr;
    int fd; 
    int err;

    if( (fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK)) < 0 ) {
        ERROR_A(c, s, data, "Cannot open: /dev/net/tun");
        return;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    
    /* Flags: 
     *      IFF_TUN    - TUN device layer 3 (no Ethernet headers) 
     *      IFF_TAP    - TAP device layer 2 
     *      IFF_NO_PI  - Do not provide packet information  
     */ 
    if (priv->tap) {
        ifr.ifr_flags = IFF_TAP;
    }
    else {
        ifr.ifr_flags = IFF_TUN;
    }

    ifr.ifr_flags += IFF_NO_PI; 
    if (priv->name[0]) {
        strncpy(ifr.ifr_name, priv->name, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        close(fd);
        ERROR_A(c, s, data, "Cannot set tun options");
        return;
    }

    priv->fd = fd;    
    strcpy(priv->name, ifr.ifr_name);
    
    if (_configure_interface(priv)) {
        close(fd);
        ERROR_A(c, s, data, "Cannot configure (set address) tun device.");
        return;
    }

    INFO("Tunnel interface: %s %s has been created.", priv->name, 
            inet_ntoa(priv->addressB));
    DEBUG("dest: %s", inet_ntoa(priv->destaddressB));
    RETURN_A(c, s, data);
}
