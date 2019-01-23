#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <climits>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"
#include "timestamps.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class Service final : public Echo::Service {
	Status echo(ServerContext *ctx, const Request *request, Reply *reply);
};

Status Service::echo(ServerContext *ctx, const Request *request, Reply *reply)
{
	// ctx->get_uuid();
	reply->set_msg(request->msg());
	return Status::OK;
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);

	std::string server_address(argv[1]);
	Service service;
	ServerBuilder builder;

	builder.AddListeningPort(server_address,
	                         grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	builder.SetMaxMessageSize(2000000000);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	server->enable_timestamps(&process_timestamps);

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever
	// return.
	server->Wait();

	return 0;
}
