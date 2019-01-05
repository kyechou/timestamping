#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"

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

	req.set_msg(input);
	Status res = stub->echo(&ctx, req, &reply);
	if (res.ok())
		return reply.msg();
	std::cerr << res.error_code() << ": " << res.error_message()
		<< std::endl;
	return "RPC failed";
}

static std::ostream &operator<< (std::ostream &os, const gpr_timespec &ts)
{
	os << ts.tv_sec << ".";
	std::streamsize width = os.width(9);
	char fill = os.fill('0');
	os << ts.tv_nsec;
	os.width(width);
	os.fill(fill);
	return os;
}

static void print_ts(void *arg, grpc::Timestamps *timestamps)
{
	std::cout << "arg = " << arg << std::endl;
	std::cout << "sendmsg(): [" << timestamps->sendmsg_seq_no << "] "
		<< timestamps->sendmsg_time << std::endl;
	std::cout << "scheduled: [" << timestamps->scheduled_seq_no << "] "
		<< timestamps->scheduled_time << std::endl;
	std::cout << "sent:      [" << timestamps->sent_seq_no << "] "
		<< timestamps->sent_time << std::endl;
	std::cout << "acked:     [" << timestamps->acked_seq_no << "] "
		<< timestamps->acked_time << std::endl;
}

int main()
{
	std::string input, reply;

	std::shared_ptr<Channel> channel =
		grpc::CreateChannel("localhost:50051",
				grpc::InsecureChannelCredentials());
	EchoClient client(channel);

	channel->enable_timestamps(&print_ts);
	while (std::getline(std::cin, input)) {
		if (input.empty())
			continue;
		reply = client.echo(input);
		std::cout << "received: " << reply << std::endl;
	}

	return 0;
}
