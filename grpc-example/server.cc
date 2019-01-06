#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"
#include "timestamps.h"

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
	const std::multimap<grpc::string_ref, grpc::string_ref> metadata
		= ctx->client_metadata();
	const grpc::string_ref uuid_str(metadata.find("rpc_uuid")->second);
	const grpc::string_ref name_str(metadata.find("func_name")->second);
	ctx->AddInitialMetadata("rpc_uuid",
			grpc::string(uuid_str.begin(), uuid_str.end()));
	ctx->AddInitialMetadata("func_name",
			grpc::string(name_str.begin(), name_str.end()));
	reply->set_msg(request->msg());
	return Status::OK;
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

	server->enable_timestamps(&process_timestamps);

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever
	// return.
	server->Wait();

	return 0;
}
