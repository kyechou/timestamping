#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/net_tstamp.h>
#include <linux/errqueue.h>
#include "utils.h"

int ts_setup(int sock)
{
	int tsflags;

	tsflags = // timestamp generation/recording
	        SOF_TIMESTAMPING_RX_HARDWARE
	        | SOF_TIMESTAMPING_RX_SOFTWARE
	        | SOF_TIMESTAMPING_TX_HARDWARE
	        | SOF_TIMESTAMPING_TX_SOFTWARE
	        | SOF_TIMESTAMPING_TX_SCHED
	        | SOF_TIMESTAMPING_TX_ACK
	        // timestamp reporting
	        | SOF_TIMESTAMPING_SOFTWARE
	        | SOF_TIMESTAMPING_RAW_HARDWARE
	        // timestamp options
	        | SOF_TIMESTAMPING_OPT_ID
	        | SOF_TIMESTAMPING_OPT_TSONLY
	        | SOF_TIMESTAMPING_OPT_TX_SWHW
	        ;

	if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPING, &tsflags,
	                sizeof(tsflags)) < 0) {
		fprintf(stderr, "setsockopt() failed: %s\n", strerr(errno));
		return 1;
	}

	return 0;
}

static inline void __print_ts(struct scm_timestamping *tss,
                              struct sock_extended_err *serr)
{
	int type, id;
	char name[64];
	struct timespec *ts;

	if (serr) {
		type = serr->ee_info;
		id = serr->ee_data;
	} else {
		type = id = -1;
	}
	switch (type) {
	case -1:
		strncpy(name, "RX    ", sizeof(name));
		break;
	case SCM_TSTAMP_SND:
		strncpy(name, "TX    ", sizeof(name));
		break;
	case SCM_TSTAMP_SCHED:
		strncpy(name, "SCHED ", sizeof(name));
		break;
	case SCM_TSTAMP_ACK:
		strncpy(name, "ACK   ", sizeof(name));
		break;
	default:
		strncpy(name, "??    ", sizeof(name));
	}

	for (int i = 0; i < 3; i += 2) {
		switch (i) {
		case 0:
			strncat(name, "SW", sizeof(name) - strlen(name) - 1);
			break;
		case 2:
			strncat(name, "HW", sizeof(name) - strlen(name) - 1);
			break;
		}

		ts = &tss->ts[i];
		if (ts->tv_sec == 0 && ts->tv_nsec == 0)
			continue;
		fprintf(stderr, "%s: [%3d] %lu.%09lu\n", name, id, ts->tv_sec,
		        ts->tv_nsec);
	}
}

static inline void printstamps(struct msghdr *msg)
{
	struct sock_extended_err *serr = NULL;
	struct scm_timestamping *tss = NULL;
	struct cmsghdr *cm;
	int batch = 0;

	for (cm = CMSG_FIRSTHDR(msg); cm; cm = CMSG_NXTHDR(msg, cm)) {
		if (cm->cmsg_level == SOL_SOCKET &&
		                cm->cmsg_type == SCM_TIMESTAMPING) {
			tss = (struct scm_timestamping *) CMSG_DATA(cm);
		} else if ((cm->cmsg_level == SOL_IP &&
		                cm->cmsg_type == IP_RECVERR) ||
		                (cm->cmsg_level == SOL_IPV6 &&
		                 cm->cmsg_type == IPV6_RECVERR)) {
			serr = (struct sock_extended_err *) CMSG_DATA(cm);
			if (serr->ee_errno != ENOMSG || serr->ee_origin !=
			                SO_EE_ORIGIN_TIMESTAMPING) {
				fprintf(stderr, "unsupported ip error: %d, "
				        "%d\n", serr->ee_errno,
				        serr->ee_origin);
				serr = NULL;
			}
		} else {
			fprintf(stderr, "unsupported cmsg level, type: %d, "
			        "%d\n", cm->cmsg_level, cm->cmsg_type);
		}

		/* print the timestamps */
		if (tss) {
			__print_ts(tss, serr);
			tss = NULL;
			serr = NULL;
			++batch;
		}
	}

	if (batch > 1)
		fprintf(stderr, "batched %d timestamps\n", batch);
}

#define CTRL_SZ 1024

int my_recv(int fd, void *buf, size_t len)
{
	int ret;
	static struct msghdr msg;
	static struct iovec iov;
	static char control[CTRL_SZ];

	memset(&msg, 0, sizeof(msg));
	memset(&iov, 0, sizeof(iov));
	memset(control, 0, sizeof(control));

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);
	iov.iov_base = buf;
	iov.iov_len = len;

	//ret = recvmsg(fd, &msg, MSG_ERRQUEUE);
	ret = recvmsg(fd, &msg, 0);
	if (ret < 0 && errno != EAGAIN) {
		fprintf(stderr, "recvmsg() failed: %s\n", strerr(errno));
		return -1;
	}
	printstamps(&msg);

	return ret;
}
