//
// Created by abclzr on 19-7-22.
//

#include <raft/Server/Server.h>

namespace raft {
    struct Server::Impl {
        std::vector<std::unique_ptr<rpc::RaftRpc::Stub>> stubs;
        std::atomic<std::size_t> cur{0};
    };

    Server::Server(const std::string &filename) {
        namespace pt = boost::property_tree;
        pt::ptree tree;
        pt::read_json(filename, tree);

        local_address = tree.get<std::string>("local");
        std::vector<std::string> srvList;
        for (auto &&srv : tree.get_child("serverList"))
            if (srv.second.get_value<std::string>() != local_address)
                srvList.emplace_back(srv.second.get_value<std::string>());

        for (const auto &srv : srvList) {
            pImpl->stubs.emplace_back(rpc::RaftRpc::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
        }
    }

    Server::~Server() {

    }

    void Server::send(const std::unique_ptr<rpc::RaftRpc::Stub> &p, const std::string &k, const std::string &v) {
        grpc::ClientContext context;
        rpc::AppendEntriesMessage request;
        rpc::Reply reply;

        rpc::Entry *entry = request.add_entries();
        entry->set_key(k); entry->set_args(v);

        p->AppendEntries(&context, request, &reply);
    }

    void Server::put(std::string k, std::string v) {
        table[k] = v;
        for (const auto &p : pImpl->stubs) {
            send(p, k, v);
        }
    }

    std::string Server::get(std::string k) {
        return table[k];
    }

    void Server::RunExternal() {
        ExternalRpcService service;
        service.bindGet(std::bind(&Server::get, this, _1));
        service.bindPut(std::bind(&Server::put, this, _1, _2));
        /*
        service.bindGet([this](std::string k) {
            return get(k);
        });
        service.bindPut([this](std::string k, std::string v) {
            put(k, v);
        });
        */
        grpc::ServerBuilder builder;
        builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        server->Wait();
    }

    void Server::RunRaft() {
        RaftRpcService service;
        service.bindAppend(std::bind(&Server::append, this, _1, _2));
        service.bindVote(std::bind(&Server::vote, this, _1, _2));
        grpc::ServerBuilder builder;
        builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        server->Wait();
    }

    void Server::append(const rpc::AppendEntriesMessage *request, rpc::Reply *reply) {
        const std::string &k = request->entries().begin()->key();
        const std::string &v = request->entries().begin()->args();
        table[k] = v;
        reply->set_ans(true);
    }

    void Server::vote(const rpc::RequestVoteMessage *request, rpc::Reply *reply) {
        reply->set_ans(true);
    }

    void Server::Run() {
        boost::thread t1(RunExternal());

    }

}