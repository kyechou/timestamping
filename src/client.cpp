#include <iostream>
#include <memory>
#include <string>
#include <cmath>
#include <unistd.h>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"
#include "timestamps.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class Client {
private:
	std::unique_ptr<Echo::Stub> stub;

public:
	Client(std::shared_ptr<Channel> channel)
		: stub(Echo::NewStub(channel)) {}

	std::string echo(const std::string &input);
};

std::string Client::echo(const std::string &input)
{
	ClientContext ctx("echo");
	Request req;
	Reply reply;

	req.set_msg(input);
	Status res = stub->echo(&ctx, req, &reply);
	if (!res.ok()) {
		std::cerr << res.error_code() << ": " << res.error_message()
			  << std::endl;
		return "RPC failed";
	}
	// ctx.get_uuid();
	return reply.msg();
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);

	std::string input, reply;
	std::shared_ptr<Channel> channel =
	        grpc::CreateChannel(argv[1],
	                            grpc::InsecureChannelCredentials());
	Client client(channel);

	channel->enable_timestamps(&process_timestamps);

	input = std::string(3000000, 'A');
	reply = client.echo(input);
	if (reply != input)
		std::cerr << "reply mismatch" << std::endl;
	input = std::string(4000000, 'A');
	reply = client.echo(input);
	if (reply != input)
		std::cerr << "reply mismatch" << std::endl;
	input = std::string(5000000, 'A');
	reply = client.echo(input);
	if (reply != input)
		std::cerr << "reply mismatch" << std::endl;

//	for (int n = 1; n < 10; ++n) {
//		input = std::string((int)pow(10.0, n), 'A');
//		reply = client.echo(input);
//		if (reply != input)
//			std::cerr << "reply mismatch" << std::endl;
//	}

	return 0;
}
