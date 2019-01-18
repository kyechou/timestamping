#include <grpcpp/grpcpp.h>
#include <mysql/mysql.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

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

static std::string to_string(const gpr_timespec &ts)
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

namespace {
class MySQLTSDB {
private:
	const char *hostname;
	const char *database;
	const char *username;
	const char *password;
	int port;
	MYSQL *conn;
	MYSQL_ROW row;

public:
	MySQLTSDB(): hostname(NULL), database("grpc_ts_demo"), username("demo"),
		password("password"), port(3306)
	{
		conn = mysql_init(NULL);
	}
	~MySQLTSDB()
	{
		mysql_close(conn);
	}
	void set_host(const char *host)
	{
		hostname = host;
	}
	bool connect();
	bool insert(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps);
};

bool MySQLTSDB::connect()
{
	if (!hostname) {
		std::cerr << "Error: hostname not set" << std::endl;
		return false;
	}
	if (mysql_real_connect(conn, hostname, username, password, database,
	                       port, NULL, 0) == NULL) {
		std::cerr << mysql_error(conn) << std::endl;
		return false;
	}
	my_bool reconnect = 1;	// enable automatic reconnection
	mysql_options(conn, MYSQL_OPT_RECONNECT, (void *)&reconnect);
	return true;
}

bool MySQLTSDB::insert(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
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
	if (mysql_query(conn, query.c_str()) != 0) {
		std::cerr << mysql_error(conn) << std::endl;
		return false;
	}
	return true;
}
}

static char usage[] = "\nUsage: ./program [<db_host>]\n";
static bool db_enabled = false;
static MySQLTSDB db;

void parse_args(int argc, char **argv)
{
	if (argc == 1)
		return;
	if (argc != 2) {
		std::cerr << usage;
		exit(1);
	}
	db_enabled = true;
	db.set_host(argv[1]);
	if (!db.connect())
		exit(1);
}

static void print_timestamps(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
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

static void store_to_db(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
{
	if (!db.insert(arg, timestamps))
		exit(1);
}

void process_timestamps(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps)
{
	if (db_enabled)
		store_to_db(arg, timestamps);
	else
		print_timestamps(arg, timestamps);
}
