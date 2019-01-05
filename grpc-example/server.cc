#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace echo;

class EchoService final : public Echo::Service {
	Status echo(ServerContext *ctx, const Request *request, Reply *reply);
};

Status EchoService::echo(ServerContext *ctx, const Request *request, Reply *reply)
{
	reply->set_msg(request->msg());
	return Status::OK;
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
	std::string server_address("0.0.0.0:50051");
	EchoService service;
	ServerBuilder builder;

	builder.AddListeningPort(server_address,
			grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	server->enable_timestamps(&print_ts);

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever
	// return.
	server->Wait();

	return 0;
}
