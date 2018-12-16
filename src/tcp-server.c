#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include "utils.h"
#include "timestamping.h"

const char usage[] = "Usage: ./tcp-server <port>\n";

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
		fprintf(stderr, "socket: %s\n", strerr(errno));
		return -1;
	}

	/* bind to server address */
	if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "bind: %s\n", strerr(errno));
		return -1;
	}

	/* listen for requests */
	if (listen(sockfd, 0) < 0) {
		fprintf(stderr, "listen: %s\n", strerr(errno));
		return -1;
	}

	return sockfd;
}

static void reaper(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
	signal(sig, reaper);
}

#define BUF_SZ 1024

static int echo(void)
{
	int len;
	char buf[BUF_SZ];
	fd_set readfs, errorfs;

	while (1) {
		FD_ZERO(&readfs);
		FD_ZERO(&errorfs);
		FD_SET(STDIN_FILENO, &readfs);
		FD_SET(STDIN_FILENO, &errorfs);
		if (select(3, &readfs, NULL, &errorfs, NULL) == -1) {
			fprintf(stderr, "select: %s\n", strerr(errno));
			return 1;
		}

		len = my_recv(STDIN_FILENO, buf, sizeof(buf) - 1, MSG_DONTWAIT);
		if (len < 0 && errno != EAGAIN && errno != ENOMSG) {
			fprintf(stderr, "my_recv (regular): "
			        "%s\n", strerr(errno));
			return 1;
		} else if (len == 0) {
			break;
		} else if (len > 0) {
			if (send(STDOUT_FILENO, buf, len, 0) < 0) {
				fprintf(stderr, "send: %s\n",
				        strerr(errno));
				return 1;
			}
		}

		len = my_recv(STDIN_FILENO, buf, sizeof(buf) - 1, MSG_ERRQUEUE);
		if (len < 0 && errno != EAGAIN && errno != ENOMSG) {
			fprintf(stderr, "my_recv (error): "
			        "%s\n", strerr(errno));
			return 1;
		}
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
		/* waiting for new connections */
		ssock = accept(msock, NULL, NULL);
		if (ssock < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "accept: %s\n", strerr(errno));
			return 1;
		}

		/* fork a process to handle the request */
		if ((childpid = fork()) < 0) {
			fprintf(stderr, "fork: %s\n", strerr(errno));
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
