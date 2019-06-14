#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include <utils/debug.h>
#include <library.h>

int frontend_udpsock_open()
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0)
	{
	 	DBG1(DBG_KNL, "failed to create socket to lookup src addresses: %s", strerror(errno));
        return -1;
	}
    return fd;
}

void frontend_udpsock_close(int fd)
{
    if (fd >= 0) close(fd);
}

int frontend_udpsock_get_host(int fd, void* ret_addr, void* owner, void* dst_addr, 
                            void (*onflush)(void *owner), 
                            void (*set_addr)(void *ret, void *sockaddr))
{
	union {
	 	struct sockaddr sockaddr;
	 	struct sockaddr_in sin;
	 	struct sockaddr_in6 sin6;
	} addr;
	socklen_t addrlen;

    host_t *dst = host_create_from_sockaddr(dst_addr);
	addrlen = *dst->get_sockaddr_len(dst);
    addr.sockaddr.sa_family = AF_UNSPEC;
	
    if (connect(fd, &addr.sockaddr, addrlen) < 0)
	{
	 	DBG1(DBG_KNL, "failed to disconnect socket: %s", strerror(errno));
        dst->destroy(dst);
	 	return FALSE;
	}

    if (onflush != NULL) onflush(owner);
	
    if (connect(fd, dst->get_sockaddr(dst), addrlen) < 0)
	{
	 	// don't report an error if we are not connected (ENETUNREACH) 
	 	if (errno != ENETUNREACH)
	 	{
	 		DBG1(DBG_KNL, "failed to connect socket: %s", strerror(errno));
	 	}
	 	else {
            //TODO, send raow event
	 	}
        dst->destroy(dst);
	 	return FALSE;
	}
    dst->destroy(dst);

	if (getsockname(fd, &addr.sockaddr, &addrlen) < 0)
	{
	 	DBG1(DBG_KNL, "failed to determine src address: %s", strerror(errno));
	 	return FALSE;
	}

    set_addr(ret_addr, (sockaddr_t*) &addr);
    return TRUE;
}
