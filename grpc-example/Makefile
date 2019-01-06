CXX = g++
CPPFLAGS += $(shell pkg-config --cflags protobuf grpc)
CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib $(shell pkg-config --libs protobuf grpc++ grpc)\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= $(shell which $(GRPC_CPP_PLUGIN))

PROTOS_PATH = .

vpath %.proto $(PROTOS_PATH)

all: client server #async_client async_client2 async_server

client: echo.pb.o echo.grpc.pb.o client.o timestamps.o
	$(CXX) -o $@ $^ $(LDFLAGS)

server: echo.pb.o echo.grpc.pb.o server.o timestamps.o
	$(CXX) -o $@ $^ $(LDFLAGS)

async_client: echo.pb.o echo.grpc.pb.o async_client.o
	$(CXX) -o $@ $^ $(LDFLAGS)

async_client2: echo.pb.o echo.grpc.pb.o async_client2.o
	$(CXX) -o $@ $^ $(LDFLAGS)

async_server: echo.pb.o echo.grpc.pb.o async_server.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	$(PROTOC) -I$(PROTOS_PATH) --grpc_out=. \
		--plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I$(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client server async_client async_client2 \
		async_server

