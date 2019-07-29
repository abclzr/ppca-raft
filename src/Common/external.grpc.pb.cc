// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: external.proto

#include "external.pb.h"
#include "external.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace raft {
namespace external {

static const char* External_method_names[] = {
  "/raft.external.External/Put",
  "/raft.external.External/Get",
  "/raft.external.External/ReplyPut",
  "/raft.external.External/ReplyGet",
};

std::unique_ptr< External::Stub> External::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< External::Stub> stub(new External::Stub(channel));
  return stub;
}

External::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_Put_(External_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Get_(External_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_ReplyPut_(External_method_names[2], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_ReplyGet_(External_method_names[3], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status External::Stub::Put(::grpc::ClientContext* context, const ::raft::external::PutRequest& request, ::raft::external::Reply* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_Put_, context, request, response);
}

void External::Stub::experimental_async::Put(::grpc::ClientContext* context, const ::raft::external::PutRequest* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_Put_, context, request, response, std::move(f));
}

void External::Stub::experimental_async::Put(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_Put_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::AsyncPutRaw(::grpc::ClientContext* context, const ::raft::external::PutRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_Put_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::PrepareAsyncPutRaw(::grpc::ClientContext* context, const ::raft::external::PutRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_Put_, context, request, false);
}

::grpc::Status External::Stub::Get(::grpc::ClientContext* context, const ::raft::external::GetRequest& request, ::raft::external::Reply* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_Get_, context, request, response);
}

void External::Stub::experimental_async::Get(::grpc::ClientContext* context, const ::raft::external::GetRequest* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_Get_, context, request, response, std::move(f));
}

void External::Stub::experimental_async::Get(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_Get_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::AsyncGetRaw(::grpc::ClientContext* context, const ::raft::external::GetRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_Get_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::PrepareAsyncGetRaw(::grpc::ClientContext* context, const ::raft::external::GetRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_Get_, context, request, false);
}

::grpc::Status External::Stub::ReplyPut(::grpc::ClientContext* context, const ::raft::external::PutReply& request, ::raft::external::Reply* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_ReplyPut_, context, request, response);
}

void External::Stub::experimental_async::ReplyPut(::grpc::ClientContext* context, const ::raft::external::PutReply* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_ReplyPut_, context, request, response, std::move(f));
}

void External::Stub::experimental_async::ReplyPut(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_ReplyPut_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::AsyncReplyPutRaw(::grpc::ClientContext* context, const ::raft::external::PutReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_ReplyPut_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::PrepareAsyncReplyPutRaw(::grpc::ClientContext* context, const ::raft::external::PutReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_ReplyPut_, context, request, false);
}

::grpc::Status External::Stub::ReplyGet(::grpc::ClientContext* context, const ::raft::external::GetReply& request, ::raft::external::Reply* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_ReplyGet_, context, request, response);
}

void External::Stub::experimental_async::ReplyGet(::grpc::ClientContext* context, const ::raft::external::GetReply* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_ReplyGet_, context, request, response, std::move(f));
}

void External::Stub::experimental_async::ReplyGet(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::raft::external::Reply* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_ReplyGet_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::AsyncReplyGetRaw(::grpc::ClientContext* context, const ::raft::external::GetReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_ReplyGet_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::raft::external::Reply>* External::Stub::PrepareAsyncReplyGetRaw(::grpc::ClientContext* context, const ::raft::external::GetReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::raft::external::Reply>::Create(channel_.get(), cq, rpcmethod_ReplyGet_, context, request, false);
}

External::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      External_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< External::Service, ::raft::external::PutRequest, ::raft::external::Reply>(
          std::mem_fn(&External::Service::Put), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      External_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< External::Service, ::raft::external::GetRequest, ::raft::external::Reply>(
          std::mem_fn(&External::Service::Get), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      External_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< External::Service, ::raft::external::PutReply, ::raft::external::Reply>(
          std::mem_fn(&External::Service::ReplyPut), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      External_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< External::Service, ::raft::external::GetReply, ::raft::external::Reply>(
          std::mem_fn(&External::Service::ReplyGet), this)));
}

External::Service::~Service() {
}

::grpc::Status External::Service::Put(::grpc::ServerContext* context, const ::raft::external::PutRequest* request, ::raft::external::Reply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status External::Service::Get(::grpc::ServerContext* context, const ::raft::external::GetRequest* request, ::raft::external::Reply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status External::Service::ReplyPut(::grpc::ServerContext* context, const ::raft::external::PutReply* request, ::raft::external::Reply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status External::Service::ReplyGet(::grpc::ServerContext* context, const ::raft::external::GetReply* request, ::raft::external::Reply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace raft
}  // namespace external

