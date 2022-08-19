#include "meloop/arrow.h"
#include "meloop/tuntap.h"
#include "meloop/io.h"
#include "meloop/logging.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>


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
	
	/* Flags: IFF_TUN	- TUN device (no Ethernet headers) 
	 *		IFF_TAP	- TAP device  
	 *
	 *		IFF_NO_PI - Do not provide packet information  
	 */ 
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI; 
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
	INFO("Tunnel interface: %s fd: %d has been opened", priv->name, priv->fd);
    RETURN_A(c, s, data);
}


/* Internet address. */
struct in_addr {
	__be32	s_addr;
};

struct sockaddr_in {
  __kernel_sa_family_t	sin_family;	/* Address family		*/
  __be16		sin_port;	/* Port number			*/
  struct in_addr	sin_addr;	/* Internet address		*/

  /* Pad to size of `struct sockaddr'. */
  unsigned char		__pad[__SOCK_SIZE__ - sizeof(short int) -
			sizeof(unsigned short int) - sizeof(struct in_addr)];
};
