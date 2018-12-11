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

static void printstamps(struct msghdr *msg)
{
	//struct timeval *tv;
	struct timespec *ts;
	struct cmsghdr *cmsg;

	for (cmsg = CMSG_FIRSTHDR(msg); cmsg; cmsg = CMSG_NXTHDR(msg, cmsg)) {
		if (cmsg->cmsg_level != SOL_SOCKET) {
			fprintf(stderr, "unsupported level %d\n",
					cmsg->cmsg_level);
			continue;
		}
		switch(cmsg->cmsg_type) {
		case SO_TIMESTAMP:
			break;
		case SO_TIMESTAMPNS:
			break;
		case SO_TIMESTAMPING:
			ts = (struct timespec *)CMSG_DATA(cmsg);
			fprintf(stderr, "%ld.%09ld:\tSW RX\n", (long)ts->tv_sec,
					(long)ts->tv_nsec);
			ts++;
			ts++; /* skip deprecated HW transformed */
			fprintf(stderr, "%ld.%09ld:\tHW RX\n", (long)ts->tv_sec,
					(long)ts->tv_nsec);
			break;
		default:
			fprintf(stderr, "unsupported type %d\n",
					cmsg->cmsg_type);
			break;
		}
	}
}

#define BUF_SZ 1024
#define CTRL_SZ 512

static int echo(void)
{
	int len;
	char buf[BUF_SZ];
	struct msghdr msg;
	struct iovec iov;
	struct {
		struct cmsghdr cmsg;
		unsigned char control[CTRL_SZ];
	} control;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &control;
	msg.msg_controllen = sizeof(control);
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	while ((len = recvmsg(STDIN_FILENO, &msg, 0)) >= 0) {
		printstamps(&msg);
		if (len > 0 && send(STDOUT_FILENO, buf, len, 0) < 0) {
			fprintf(stderr, "send() failed: %s\n", strerr(errno));
			return 1;
		}
	}

	if (len < 0) {
		fprintf(stderr, "recvmsg() failed: %s\n", strerr(errno));
		return 1;
	}

	return 0;
}

static inline int ts_setup(int sock)
{
	int tsflags; //, enabled = 1;

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

	/*
	if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMP, &enabled,
	                sizeof(enabled)) < 0) {
		fprintf(stderr, "setsockopt() failed: %s\n", strerr(errno));
		return 1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled,
	                sizeof(enabled)) < 0) {
		fprintf(stderr, "setsockopt() failed: %s\n", strerr(errno));
		return 1;
	}
	*/
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
