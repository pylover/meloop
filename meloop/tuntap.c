#include "meloop/arrow.h"
#include "meloop/tuntap.h"
#include "meloop/io.h"
#include "meloop/addr.h"
#include "meloop/logging.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>


static int
_contigure_interface(struct tunS *tun) {
    struct ifreq ifr;
    struct sockaddr_in *tmp;

	/* Interface name */
	strncpy(ifr.ifr_name, tun->name, IFNAMSIZ-1);
    
    /* Netmask */
    if (meloop_in_addr_parse(tun->netmask, &tun->netmaskB)) {
        return ERR;
    }
	tmp = (struct sockaddr_in*) &ifr.ifr_netmask;
	tmp->sin_family = AF_INET;
    tmp->sin_addr = tun->netmaskB;
    if (ioctl(tun->fd, SIOCSIFNETMASK, &ifr)) {
        return ERR;
    }

  	// /* Set IPv4 address */
	// ifr.ifr_addr.sa_family = AF_INET;
	// ifr.ifr_addr.sin_addr = tun->address;
    
    return OK;
}

// struct ifreq {
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

// /* Internet address. */
// struct in_addr {
// 	__be32	s_addr;
// };
// 
// struct sockaddr_in {
//   __kernel_sa_family_t	sin_family;	/* Address family		*/
//   __be16		sin_port;	/* Port number			*/
//   struct in_addr	sin_addr;	/* Internet address		*/
// 
//   /* Pad to size of `struct sockaddr'. */
//   unsigned char		__pad[__SOCK_SIZE__ - sizeof(short int) -
// 			sizeof(unsigned short int) - sizeof(struct in_addr)];
// };


void 
tunopenA(struct circuitS *c, void *s, void *data) {
    struct tunS *priv = meloop_priv_ptr(c);
	struct ifreq ifr;
	int fd; 
    int err;

	if( (fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK)) < 0 ) {
		ERROR_A(c, s, data, "Cannot open: /dev/net/tun");
		return;
	}
	
	memset(&ifr, 0, sizeof(ifr));
	
	/* Flags: 
     *      IFF_TUN	- TUN device layer 3 (no Ethernet headers) 
	 *		IFF_TAP	- TAP device layer 2 
	 *
	 *		IFF_NO_PI - Do not provide packet information  
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
    
    if (_contigure_interface(priv)) {
		close(fd);
		ERROR_A(c, s, data, "Cannot configure (set address) tun device.");
        return;
    }

	INFO("Tunnel interface: %s fd: %d has been opened", priv->name, priv->fd);
    RETURN_A(c, s, data);
}
