#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <locale.h>
#include <errno.h>
#include <unistd.h>
#include <linux/net_tstamp.h>
#include <linux/errqueue.h>
#include <sys/time.h>

const char usage[] = "Usage: ./echo-server <port>\n";

static const char *strerr(int errnum)
{
	const char *msg;
	locale_t locale;

	locale = newlocale(LC_ALL_MASK, "", 0);
	msg = strerror_l(errnum, locale);
	freelocale(locale);
	return msg;
}

static int passiveTCP(int port)
{
	int sockfd;
	struct sockaddr_in addr;

	/* set up server socket addr */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	/* open a TCP socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stderr, "socket() failed: %s\n", strerr(errno));
		return -1;
	}

	/* bind to server address */
	if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "bind() failed: %s\n", strerr(errno));
		return -1;
	}

	/* listen for requests */
	if (listen(sockfd, 0) < 0) {
		fprintf(stderr, "listen() failed: %s\n", strerr(errno));
		return -1;
	}

	return sockfd;
}

static void reaper(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
	signal(sig, reaper);
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

#define BUF_SZ 1024
#define CTRL_SZ 1024

static int my_recv(int fd, void *buf, size_t len)
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

static int echo(void)
{
	int len;
	char buf[BUF_SZ];

	while ((len = my_recv(STDIN_FILENO, buf, sizeof(buf))) >= 0) {
		if (len > 0 && send(STDOUT_FILENO, buf, len, 0) < 0) {
			fprintf(stderr, "send() failed: %s\n", strerr(errno));
			return 1;
		}
	}

	if (len < 0) {
		fprintf(stderr, "my_recv() failed: %s\n", strerr(errno));
		return 1;
	}

	return 0;
}

static inline int ts_setup(int sock)
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

int main(int argc, char **argv)
{
	char *end;
	int msock, ssock;
	long port;
	pid_t childpid;

	/* port number */
	if (argc != 2 || !(port = strtol(argv[1], &end, 10)) || *end != '\0') {
		fputs(usage, stderr);
		return 1;
	}

	/* TCP connection */
	if ((msock = passiveTCP(port)) < 0)
		return 1;

	signal(SIGCHLD, reaper);
	while (1) {
		/* accept connection request */
		ssock = accept(msock, NULL, NULL);
		if (ssock < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "accept() failed: %s\n", strerr(errno));
			return 1;
		}

		/* fork another process to handle the request */
		if ((childpid = fork()) < 0) {
			fprintf(stderr, "fork() failed: %s\n", strerr(errno));
			return 1;
		} else if (childpid == 0) {
			if (ts_setup(ssock) != 0)
				return 1;
			dup2(ssock, STDIN_FILENO);
			dup2(ssock, STDOUT_FILENO);
			close(msock);
			close(ssock);
			return echo();
		}
		close(ssock);
	}

	return 0;
}
