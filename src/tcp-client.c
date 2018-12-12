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
		fprintf(stderr, "socket failed: %s\n", strerr(errno));
		return -1;
	}

	/* connect to the server */
	if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "connect failed: %s\n", strerr(errno));
		return -1;
	}

	return sockfd;
}

#define BUF_SZ 1024

static int echo(int sock)
{
	char buf[BUF_SZ];
	fd_set rfds, afds;
	int nfds = getdtablesize();

	FD_ZERO(&afds);
	FD_SET(sock, &afds);
	FD_SET(STDIN_FILENO, &afds);

	while (1) {
		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(nfds, &rfds, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "select failed: %s\n", strerr(errno));
			return 1;
		}
		if (FD_ISSET(STDIN_FILENO, &rfds)) {
			char *p;
			p = fgets(buf, sizeof(buf), stdin);
			if (ferror(stdin)) {
				clearerr(stdin);
				fprintf(stderr, "fgets failed\n");
				return 1;
			} else if (feof(stdin)) {
				clearerr(stdin);
				break;
			}
			if (!p)
				continue;
			if (send(sock, buf, strlen(buf), 0) < 0) {
				fprintf(stderr, "send failed: %s\n",
				        strerr(errno));
				return 1;
			}
		}
		if (FD_ISSET(sock, &rfds)) {
			int len;
			len = my_recv(sock, buf, sizeof(buf));
			if (len < 0) {
				fprintf(stderr, "my_recv failed: %s\n",
				        strerr(errno));
				return 1;
			}
			buf[len] = 0;
			fputs(buf, stdout);
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
		fprintf(stderr, "gethostbyname failed\n");
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
