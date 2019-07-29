//
// Created by abclzr on 19-7-22.
//

#include <raft/Server/Server.h>

namespace raft {
    struct Server::Impl {
        std::vector<std::unique_ptr<rpc::RaftRpc::Stub>> stubs;
        std::vector<std::unique_ptr<rpc::External::Stub>> Exstubs;
        std::atomic<std::size_t> cur{0};
    };

    Server::Server(const std::string &filename, uint64_t _clustnum, const std::string &Clientaddr) : clustsize(_clustnum) {
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

        pImpl->Exstubs.emplace_back(rpc::External::NewStub((grpc::CreateChannel(
                    Clientaddr, grpc::InsecureChannelCredentials()))));

        work_is_done = false;
        log.emplace_back(0, 0, "", "");

        //TODO:: add heart-bind
        heart.Run();
    }

    void Server::put(std::string k, std::string v) {
    }

    void Server::get(std::string k) {
    }

    void Server::RunExternal() {
        ExternalRpcService service;
            service.bindGet([this](std::string k) {
                get(k);
            });
            service.bindPut([this](std::string k, std::string v) {
                put(k, v);
            });
            grpc::ServerBuilder builder;
            builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            serverExternal = builder.BuildAndStart();
            serverExternal->Wait();
    }

    void Server::RunRaft() {
            RaftRpcService service;
            service.bindrequestAE([this](const rpc::RequestAppendEntries *request, rpc::Reply *reply) {
                requestAE(request, reply);
            });
            service.bindrequestV([this](const rpc::RequestVote *request, rpc::Reply *reply) {
                requestV(request, reply);
            });
            service.bindreplyAE([this](const rpc::ReplyAppendEntries *request, rpc::Reply *reply) {
                replyAE(request, reply);
            });
            service.bindreplyV([this](const rpc::ReplyVote *request, rpc::Reply *reply) {
                replyV(request, reply);
            });
            grpc::ServerBuilder builder;
            builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            serverRaft = builder.BuildAndStart();
            serverRaft->Wait();
    }

    void Server::RunProcess() {
        boost::unique_lock<boost::mutex> lock(mu);
        while (!work_is_done) {
            cv.wait(lock, [this](){return !q.empty();});
            while (!q.empty()) {

            }
        }
    }

    void Server::Run() {
        boost::function0<void> f1 = boost::bind(&Server::RunExternal, this);
        boost::function0<void> f2 = boost::bind(&Server::RunRaft, this);
        boost::function0<void> f3 = boost::bind(&Server::RunProcess, this);
        t1 = boost::thread(f1);
        t2 = boost::thread(f2);
        t3 = boost::thread(f3);
        heart.Run();
    }
    
    void Server::Stop() {
        serverExternal->Shutdown();
        serverRaft->Shutdown();
        work_is_done = true;
        t1.interrupt();
        t2.interrupt();
        t3.interrupt();
        if (t1.joinable()) t1.join();
        if (t2.joinable()) t2.join();
        if (t3.joinable()) t3.join();
        heart.Stop();
    }

    Server::~Server() {
        Stop();
    }

    uint64_t Server::get_lastlogindex() {
        return (--log.end())->index;
    }

    uint64_t Server::get_lastlogterm() {
        return (--log.end())->term;
    }

    Server::LogEntry::LogEntry(uint64_t _term, uint64_t _index, const std::string &_key, const std::string &_args)
         : term(_term), index(_index), key(_key), args(_args) {}

}