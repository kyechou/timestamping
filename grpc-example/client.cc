#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"
#include "timestamps.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace echo;

class EchoClient {
private:
	std::unique_ptr<Echo::Stub> stub;

public:
	EchoClient(std::shared_ptr<Channel> channel)
		: stub(Echo::NewStub(channel)) {}

	std::string echo(const std::string &input);
};

std::string EchoClient::echo(const std::string &input)
{
	ClientContext ctx;
	Request req;
	Reply reply;

	ctx.AddMetadata("rpc_uuid", "12345");
	ctx.AddMetadata("func_name", "echo");
	req.set_msg(input);
	Status res = stub->echo(&ctx, req, &reply);
	if (res.ok())
		return reply.msg();
	std::cerr << res.error_code() << ": " << res.error_message()
		<< std::endl;
	return "RPC failed";
}

int main()
{
	std::string input, reply;

	std::shared_ptr<Channel> channel =
		grpc::CreateChannel("localhost:50051",
				grpc::InsecureChannelCredentials());
	EchoClient client(channel);

	channel->enable_timestamps(&process_timestamps);

	while (std::getline(std::cin, input)) {
		if (input.empty())
			continue;
		reply = client.echo(input);
		std::cout << "received: " << reply << std::endl;
	}

	return 0;
}
