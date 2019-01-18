#ifndef TIMESTAMPS_H
#define TIMESTAMPS_H

#include <grpcpp/timestamps.h>

void parse_args(int argc, char **argv);
void process_timestamps(grpc::TimestampsArgs *arg, grpc::Timestamps *timestamps);

#endif
