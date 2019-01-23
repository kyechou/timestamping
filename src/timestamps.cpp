#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <grpcpp/grpcpp.h>

namespace {

std::string to_string(const gpr_timespec &ts)
{
	std::ostringstream oss;
	oss << ts.tv_sec;
	if (ts.tv_nsec) {
		oss << ".";
		oss.width(9);
		oss.fill('0');
		oss << ts.tv_nsec;
	}
	return oss.str();
}

std::string get_qstr(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
{
	std::string query(
	        "INSERT INTO Timestamps (uuid, name, rpc_type, peer, seq, "
	        "sendmsg_time, scheduled_time, sent_time, received_time, "
	        "acked_time) VALUES ("
	);
	query +=  "'" + arg->rpc_uuid  + "', "
	          + "'" + arg->func_name + "', "
	          + "'" + arg->rpc_type  + "', "
	          + "'" + arg->peer      + "', "
	          + std::to_string(arg->seq_no) + ", "
	          + to_string(timestamps->sendmsg_time) + ", "
	          + to_string(timestamps->scheduled_time) + ", "
	          + to_string(timestamps->sent_time) + ", "
	          + to_string(timestamps->received_time) + ", "
	          + to_string(timestamps->acked_time) + ");";
	return query;
}

std::ostream &operator<< (std::ostream &os, const gpr_timespec &ts)
{
	os << ts.tv_sec << ".";
	std::streamsize width = os.width(9);
	char fill = os.fill('0');
	os << ts.tv_nsec;
	os.width(width);
	os.fill(fill);
	return os;
}

void print_timestamps(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
{
	std::cout << "===========================" << std::endl;
	if (arg) {
		std::cout << "UUID:      " << arg->rpc_uuid << std::endl;
		std::cout << "Name:      " << arg->func_name << std::endl;
		std::cout << "Type:      " << arg->rpc_type << std::endl;
		std::cout << "Peer:      " << arg->peer << std::endl;
		std::cout << "Seq No:    " << arg->seq_no << std::endl;
	}
	std::cout << "sendmsg(): " << timestamps->sendmsg_time << std::endl;
	std::cout << "scheduled: " << timestamps->scheduled_time << std::endl;
	std::cout << "sent:      " << timestamps->sent_time << std::endl;
	std::cout << "received:  " << timestamps->received_time << std::endl;
	std::cout << "acked:     " << timestamps->acked_time << std::endl;
}

const char usage[] = "\n"
	"Usage: ./program [OPTIONS]\n"
	"Options:\n"
	"    -p        Print to standard output\n";
int dumpfd = -1;
char dumpfile[] = "tsdump.XXXXXX.sql";

}  // namespace

void parse_args(int argc, char **argv)
{
	if (argc == 1) {
		dumpfd = mkostemps(dumpfile, 4, O_TRUNC);
		if (dumpfd == -1) {
			std::cerr << "Failed to create " << dumpfile
				<< std::endl;
			exit(1);
		}
	} else if (argc != 2 || strncmp(argv[1], "-p", 3) != 0) {
		std::cerr << usage;
		exit(1);
	}
}

void process_timestamps(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
{
	if (dumpfd < 0) {
		print_timestamps(arg, timestamps);
	} else {
		std::string query = get_qstr(arg, timestamps) + "\n";
		if (write(dumpfd, query.c_str(), query.size()) == -1) {
			std::cerr << "Failed to write to " << dumpfile
				<< std::endl;
		}
	}
}
