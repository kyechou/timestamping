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
#include "utils.hpp"
#include "timestamping.hpp"

const char usage[] = "Usage: ./udp-server <port>\n";

static int serverUDP(int port)
{
	int sockfd;
	struct sockaddr_in addr;

	/* set up server socket addr */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	/* open a UDP socket */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "socket: %s\n", strerr(errno));
		return -1;
	}

	/* bind to server address */
	if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "bind: %s\n", strerr(errno));
		return -1;
	}

	return sockfd;
}

#define BUF_SZ 1024

int main(int argc, char **argv)
{
	char *end, buf[BUF_SZ];
	int sock, port, len;
	struct sockaddr addr;
	socklen_t addrlen;
	fd_set readfs, errorfs;

	/* port number */
	if (argc != 2 || !(port = strtol(argv[1], &end, 10)) || *end != '\0') {
		fputs(usage, stderr);
		return 1;
	}

	/* UDP socket */
	if ((sock = serverUDP(port)) < 0)
		return 1;
	if (ts_setup(sock) != 0)
		return 1;
	dup2(sock, STDIN_FILENO);
	dup2(sock, STDOUT_FILENO);
	close(sock);

	while (1) {
		FD_ZERO(&readfs);
		FD_ZERO(&errorfs);
		FD_SET(STDIN_FILENO, &readfs);
		FD_SET(STDIN_FILENO, &errorfs);
		if (select(3, &readfs, NULL, &errorfs, NULL) == -1) {
			fprintf(stderr, "select: %s\n", strerr(errno));
			return 1;
		}

		addrlen = sizeof(addr);
		memset(&addr, 0, addrlen);
		len = my_recvfrom(STDIN_FILENO, buf, sizeof(buf) - 1,
		                  MSG_DONTWAIT, &addr, &addrlen);
		if (len < 0 && errno != EAGAIN && errno != ENOMSG) {
			fprintf(stderr, "my_recvfrom (regular): %s\n",
			        strerr(errno));
			return 1;
		} else if (len == 0) {
			break;
		} else if (len > 0) {
			if (sendto(STDOUT_FILENO, buf, len, 0,
			                &addr, addrlen) < 0) {
				fprintf(stderr, "sendto: %s\n", strerr(errno));
				return 1;
			}
		}

		len = my_recv(STDIN_FILENO, buf, sizeof(buf) - 1, MSG_ERRQUEUE);
		if (len < 0 && errno != EAGAIN && errno != ENOMSG) {
			fprintf(stderr, "my_recv (error): %s\n", strerr(errno));
			return 1;
		}
	}

	return 0;
}
