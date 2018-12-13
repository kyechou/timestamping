#ifndef _TIMESTAMPING_H
#define _TIMESTAMPING_H

extern int ts_setup(int sock);
extern int my_recv(int fd, void *buf, size_t len, int flags);

#endif
