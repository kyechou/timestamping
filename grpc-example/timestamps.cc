#include <grpcpp/grpcpp.h>
#include <iostream>

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

void process_timestamps(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
{
	/* print it out for now */
	if (arg) {
		std::cout << "UUID:   " << arg->rpc_uuid << std::endl;
		std::cout << "Name:   " << arg->func_name << std::endl;
		std::cout << "Type:   " << arg->rpc_type << std::endl;
		std::cout << "Dest:   " << arg->pkt_dest << std::endl;
		std::cout << "Seq No: " << arg->seq_no << std::endl;
	}
	std::cout << "sendmsg(): " << timestamps->sendmsg_time << std::endl;
	std::cout << "scheduled: " << timestamps->scheduled_time << std::endl;
	std::cout << "sent:      " << timestamps->sent_time << std::endl;
	std::cout << "received:  " << timestamps->received_time << std::endl;
	std::cout << "acked:     " << timestamps->acked_time << std::endl;
}
