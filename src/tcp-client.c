#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include "utils.h"
#include "timestamping.h"

const char usage[] = "Usage: ./tcp-client <host> <port>\n";

static int activeTCP(struct in_addr inet_addr, int port)
{
	int sockfd;
	struct sockaddr_in addr;

	/* set up target socket addr */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr = inet_addr;
	addr.sin_port = htons(port);

	/* open a TCP socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stderr, "socket: %s\n", strerr(errno));
		return -1;
	}

	/* connect to the server */
	if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "connect: %s\n", strerr(errno));
		return -1;
	}

	return sockfd;
}

#define BUF_SZ 1024

static int echo(int sock)
{
	int len;
	char buf[BUF_SZ];
	fd_set readfs, errorfs;

	while (1) {
		FD_ZERO(&readfs);
		FD_ZERO(&errorfs);
		FD_SET(STDIN_FILENO, &readfs);
		FD_SET(sock, &readfs);
		FD_SET(sock, &errorfs);
		if (select(sock + 1, &readfs, NULL, &errorfs, NULL) == -1) {
			fprintf(stderr, "select: %s\n", strerr(errno));
			return 1;
		}
		if (FD_ISSET(STDIN_FILENO, &readfs)) {
			if (!fgets(buf, sizeof(buf), stdin))
				break;
			if (send(sock, buf, strlen(buf), 0) < 0) {
				fprintf(stderr, "send: %s\n", strerr(errno));
				return 1;
			}
		}
		if (FD_ISSET(sock, &readfs) || FD_ISSET(sock, &errorfs)) {
			len = my_recv(sock, buf, sizeof(buf) - 1, MSG_DONTWAIT);
			if (len < 0 && errno != EAGAIN) {
				fprintf(stderr, "my_recv (regular): "
				        "%s\n", strerr(errno));
				return 1;
			} else if (len == 0) {
				break;
			}

			len = my_recv(sock, buf, sizeof(buf) - 1, MSG_ERRQUEUE);
			if (len < 0 && errno != EAGAIN) {
				fprintf(stderr, "my_recv (error): "
					"%s\n", strerr(errno));
				return 1;
			}
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	char *end;
	struct in_addr addr;
	int sock, port;
	struct hostent *host;

	/* port number */
	if (argc != 3 || !(port = strtol(argv[2], &end, 10)) || *end != '\0') {
		fputs(usage, stderr);
		return 1;
	}

	/* hostname/addresss */
	if ((host = gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "gethostbyname\n");
		return 1;
	}
	addr = *(struct in_addr *)host->h_addr_list[0];

	/* open TCP connection */
	if ((sock = activeTCP(addr, port)) < 0)
		return 1;

	if (ts_setup(sock) != 0)
		return 1;
	return echo(sock);
}
