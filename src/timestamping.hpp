#ifndef _TIMESTAMPING_H
#define _TIMESTAMPING_H

#define CTRL_SZ 1024

extern int ts_setup(int sock);
extern int my_recv(int fd, void *buf, size_t len, int flags);
extern int my_recvfrom(int fd, void *buf, size_t len, int flags,
                       struct sockaddr *src_addr, socklen_t *addrlen);

#endif
