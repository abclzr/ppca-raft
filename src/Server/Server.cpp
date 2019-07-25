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

    void Server::sendAppendEntriesMessage(const std::unique_ptr<rpc::RaftRpc::Stub> &p, const std::string &k, const std::string &v) {
        grpc::ClientContext context;
        rpc::AppendEntriesMessage request;
        rpc::Reply reply;

        rpc::Entry *entry = request.add_entries();
        entry->set_key(k); entry->set_args(v);

        reply.set_ans(false);

        p->AppendEntries(&context, request, &reply);
    }

    void Server::put(std::string k, std::string v) {
        table[k] = v;
        for (const auto &p : pImpl->stubs) {
            sendAppendEntriesMessage(p, k, v);
        }
    }

    std::string Server::get(std::string k) {
        return table[k];
    }

    void Server::RunExternal() {
        try {
            ExternalRpcService service;
            /*
            service.bindGet(std::bind(&Server::get, this, _1));
            service.bindPut(std::bind(&Server::put, this, _1, _2));
            */
            service.bindGet([this](std::string k) {
                return get(k);
            });
            service.bindPut([this](std::string k, std::string v) {
                put(k, v);
            });
            grpc::ServerBuilder builder;
            builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            serverExternal = builder.BuildAndStart();
            serverExternal->Wait();
        } catch (...) {
            return;
        }
    }

    void Server::RunRaft() {
        try {
            RaftRpcService service;
            /*
            service.bindAppend(std::bind(&Server::append, this, _1, _2));
            service.bindVote(std::bind(&Server::vote, this, _1, _2));
            */
            service.bindAppend([this](const rpc::AppendEntriesMessage *request, rpc::Reply *reply) {
                append(request, reply);
            });
            service.bindVote([this](const rpc::RequestVoteMessage *request, rpc::Reply *reply) {
                vote(request, reply);
            });
            grpc::ServerBuilder builder;
            builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            serverRaft = builder.BuildAndStart();
            serverRaft->Wait();
        } catch (...) {
            return;
        }
    }

    void Server::append(const rpc::AppendEntriesMessage *request, rpc::Reply *reply) {
        try {
            //TODO: add election timeout interruption

            boost::lock_guard<boost::mutex> lock(mu);
            reply->set_term(currentTerm);

            if (request->term() < currentTerm) {
                reply->set_ans(false);
                return;
            }
            if (request->term() > currentTerm) currentTerm = request->term();

            auto it = findLog(request->prevlogindex());
            if (it == log.end() || it->term != request->prevlogterm()) {
                reply->set_ans(false);
                return;
            }

            uint64_t id = it->index;
            while (it != (--log.end())) log.pop_back();
            for (const auto &leader_it : request->entries()) {
                ++id;
                log.emplace_back(leader_it.term(), id, leader_it.key(), leader_it.args());
            }

            if (request->leadercommit() > commitIndex)
                commitIndex = std::min((uint64_t) request->leadercommit(), id);
            reply->set_ans(true);
        } catch (...) {
            mu.unlock();
        }
    }

    void Server::vote(const rpc::RequestVoteMessage *request, rpc::Reply *reply) {
        try {
            //TODO: add election timeout interruption

            boost::lock_guard<boost::mutex> lock(mu);
            reply->set_term(currentTerm);

            if (currentTerm < request->term()) currentTerm = request->term();

            if (request->lastlogterm() < get_lastlogterm()
                || ((request->lastlogterm() == get_lastlogterm()) && (request->lastlogindex() < get_lastlogindex()))) {
                reply->set_ans(false);
                return;
            }

            if (votedFor.empty()) {
                votedFor = request->candidateid();
                reply->set_ans(true);
            } else {
                reply->set_ans(false);
            }
        } catch (...) {
            mu.unlock();
        }
    }

    void Server::Run() {
        boost::function0<void> f1 = boost::bind(&Server::RunExternal, this);
        boost::function0<void> f2 = boost::bind(&Server::RunRaft, this);
        t1 = boost::thread(f1);
        t2 = boost::thread(f2);
        heart.Run();
    }
    
    void Server::Stop() {
        serverExternal->Shutdown();
        serverRaft->Shutdown();
        t1.interrupt();
        t2.interrupt();
        if (t1.joinable()) t1.join();
        if (t2.joinable()) t2.join();
    }

    Server::~Server() {
        Stop();
    }

    std::vector<raft::Server::LogEntry, std::allocator<raft::Server::LogEntry>>::iterator Server::findLog(uint64_t prevLogIndex) {
        for (auto i = log.begin(); i != log.end(); ++i)
            if (i->index == prevLogIndex)
                return i;
        return log.end();
    }

    uint64_t Server::get_lastlogindex() {
        if (log.empty()) return 0;
        else return (--log.end())->index;
    }

    uint64_t Server::get_lastlogterm() {
        if (log.empty()) return 0;
        else return (--log.end())->term;
    }

    Server::LogEntry::LogEntry(uint64_t _term, uint64_t _index, const std::string &_key, const std::string &_args)
         : term(_term), index(_index), key(_key), args(_args) {}



}