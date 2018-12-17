#include <iostream>
#include <string>
#include "rpc/client.h"

const std::string usage = "Usage: ./rpc-client <host> <port>\n";

int main(int argc, char **argv)
{
	int port;
	char *end;
	std::string input, result;

	/* port number */
	if (argc != 3 || !(port = std::strtol(argv[2], &end, 10))
	                || *end != '\0') {
		std::cerr << usage;
		return 1;
	}

	/* RPC client */
	rpc::client client(argv[1], port);

	while (std::getline(std::cin, input)) {
		if (input.empty())
			continue;
		result = client.call("echo", input).as<std::string>();
		std::cout << result << std::endl;
	}

	return 0;
}
