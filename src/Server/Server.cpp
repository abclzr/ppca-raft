//
// Created by abclzr on 19-7-22.
//

#include <raft/Server/Server.h>

namespace raft {
    struct Server::Impl {
        std::vector<std::unique_ptr<rpc::RaftRpc::Stub>> stubs;
        std::atomic<std::size_t> cur{0};
    };

    Server::Server(const std::string &filename, uint64_t _clustnum) : clustsize(_clustnum) {
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

        log.emplace_back(0, 0, "", "");

        heart.bindElection([this](uint64_t TIME) {return election(TIME);});
        heart.bindDeclareleader([this]() {declareleader();});
        heart.bindSendHeartBeat([this]() {sendHeartBeat();});
        heart.Run();
    }

    std::unique_ptr<rpc::Reply> Server::sendAppendEntriesMessage(const std::unique_ptr<rpc::RaftRpc::Stub> &p, uint64_t index) {
        grpc::ClientContext context;
        rpc::AppendEntriesMessage request;
        auto reply = new rpc::Reply;

        request.set_term(currentTerm);
        request.set_leaderid(local_address);
        request.set_prevlogterm(log[index - 1].term);
        request.set_prevlogindex(log[index - 1].index);
        while (index < log.size()) {
            rpc::Entry *entry = request.add_entries();
            entry->set_term(log[index].term);
            entry->set_key(log[index].key);
            entry->set_args(log[index].args);
            ++index;
        }
        request.set_leadercommit(commitIndex);


        reply->set_ans(false);

        p->AppendEntries(&context, request, reply);
        return std::unique_ptr<rpc::Reply>(reply);
    }

    std::unique_ptr<rpc::Reply> Server::requestVote(const std::unique_ptr<raft::rpc::RaftRpc::Stub> &p) {
        grpc::ClientContext context;
        rpc::RequestVoteMessage request;
        auto reply = new rpc::Reply;

        request.set_term(currentTerm);
        request.set_candidateid(local_address);
        request.set_lastlogterm(get_lastlogterm());
        request.set_lastlogindex(get_lastlogindex());

        reply->set_ans(false);

        p->RequestVote(&context, request, reply);
        return std::unique_ptr<rpc::Reply>(reply);
    }

    void Server::put(std::string k, std::string v) {
        log.emplace_back(LogEntry(currentTerm, log.size() + 1, k, v));
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
            heart.interrupt();

            boost::lock_guard<boost::mutex> lock(mu);
            reply->set_term(currentTerm);
            votedFor.clear();

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
            return;
        }
    }

    void Server::vote(const rpc::RequestVoteMessage *request, rpc::Reply *reply) {
        try {
            //TODO: add election timeout interruption
            heart.interrupt();

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
            return;
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
        heart.Stop();
    }

    Server::~Server() {
        Stop();
    }

    std::vector<raft::Server::LogEntry, std::allocator<raft::Server::LogEntry>>::iterator Server::findLog(uint64_t prevLogIndex) {
        return log.begin() + prevLogIndex;
    }

    uint64_t Server::get_lastlogindex() {
        return (--log.end())->index;
    }

    uint64_t Server::get_lastlogterm() {
        return (--log.end())->term;
    }

    bool Server::election(uint64_t TIME) {
        boost::lock_guard<boost::mutex> lock(mu);
        votedFor = local_address;
        uint32_t cnt = 0;
        for (const auto &p : pImpl->stubs) {
            std::unique_ptr<rpc::Reply> reply = requestVote(p);
            if (reply->ans()) ++cnt;
        }
        return cnt > clustsize / 2;
    }

    void Server::declareleader() {
        boost::lock_guard<boost::mutex> lock(mu);
        for (const auto &p : pImpl->stubs) {
            std::unique_ptr<rpc::Reply> reply = sendAppendEntriesMessage(p, log.size());
        }
    }

    void Server::sendHeartBeat() {
        boost::lock_guard<boost::mutex> lock(mu);
        for (auto &i : nextIndex) i = log.size();
        for (auto &i : matchIndex) i = 0;
        auto it_nextIndex = nextIndex.begin();
        auto it_matchIndex = matchIndex.begin();
        for (const auto &p : pImpl->stubs) {
            std::unique_ptr<rpc::Reply> reply = sendAppendEntriesMessage(p, *it_nextIndex);
            if (reply->ans()) {
                *it_nextIndex = log.size();
                *it_matchIndex = log.size() - 1;
            } else {
                if (*it_nextIndex > 0) --(*it_nextIndex);
            }
        }
    }

    Server::LogEntry::LogEntry(uint64_t _term, uint64_t _index, const std::string &_key, const std::string &_args)
         : term(_term), index(_index), key(_key), args(_args) {}

}