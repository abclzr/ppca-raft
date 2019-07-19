#!/usr/bin/env bash

protoc -I=. --cpp_out=../src/Common/ external.proto
protoc -I=. --grpc_out=../src/Common/ \
  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` external.proto
mv ../src/Common/external.grpc.pb.h ../include/raft/Common/
mv ../src/Common/external.pb.h ../include/raft/Common/
