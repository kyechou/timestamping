#include <iostream>
#include <string>
#include "rpc/server.h"

const std::string usage = "Usage: ./rpc-server <port>\n";

const std::string &echo(const std::string &s)
{
	return s;
}

int main(int argc, char **argv)
{
	int port;
	char *end;

	/* port number */
	if (argc != 2 || !(port = std::strtol(argv[1], &end, 10))
	                || *end != '\0') {
		std::cerr << usage;
		return 1;
	}

	/* RPC server */
	rpc::server srv(port);
	srv.bind("echo", &echo);

	/* server loop */
	srv.run();

	return 0;
}
