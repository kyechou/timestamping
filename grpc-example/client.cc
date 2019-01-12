#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"
#include "timestamps.h"

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
	ClientContext ctx;
	Request req;
	Reply reply;

	ctx.set_timestamps_metadata("echo");
	req.set_msg(input);
	Status res = stub->echo(&ctx, req, &reply);
	if (res.ok())
		return reply.msg();
	std::cerr << res.error_code() << ": " << res.error_message()
		<< std::endl;
	return "RPC failed";
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);

	std::string input, reply;
	std::shared_ptr<Channel> channel =
		grpc::CreateChannel("localhost:50051",
				grpc::InsecureChannelCredentials());
	Client client(channel);

	channel->enable_timestamps(&process_timestamps);

	for (int i = 0; i < 50; ++i) {
		input = std::string(100, 'A');
		reply = client.echo(input);
		if (reply != input)
			std::cerr << "reply mismatch" << std::endl;
	}

	return 0;
}
