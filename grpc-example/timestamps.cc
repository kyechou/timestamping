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
	std::cout << "destination = " << arg->pkt_dest << std::endl;
	std::cout << "sendmsg(): [" << arg->seq_no << "] "
		<< timestamps->sendmsg_time << std::endl;
	std::cout << "scheduled: [" << arg->seq_no << "] "
		<< timestamps->scheduled_time << std::endl;
	std::cout << "sent:      [" << arg->seq_no << "] "
		<< timestamps->sent_time << std::endl;
	std::cout << "acked:     [" << arg->seq_no << "] "
		<< timestamps->acked_time << std::endl;
}
