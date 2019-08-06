//
// Created by abclzr on 19-7-22.
//

#include <raft/Server/Server.h>

namespace raft {
    struct Server::Impl {
        std::vector<std::unique_ptr<rpc::RaftRpc::Stub>> stubs;
        std::vector<std::unique_ptr<external::External::Stub>> redirectStubs;
        std::vector<std::unique_ptr<external::External::Stub>> clientStubs;
        std::atomic<std::size_t> cur{0};
    };

    Server::Server(const std::string &filename) : pImpl(std::make_unique<Impl>()) {
        namespace pt = boost::property_tree;
        pt::ptree tree;
        pt::read_json(filename, tree);

        local_address = tree.get_child("local.address").get_value<std::string>();
        external_local_address = tree.get_child("local.externalAddress").get_value<std::string>();

        std::vector<std::string> srvList;
        std::vector<std::string> exSrvList;
        std::vector<std::string> cltList;
        for (auto &&srv : tree.get_child("serverList"))
            if (srv.second.get_value<std::string>() != local_address)
                srvList.emplace_back(srv.second.get_value<std::string>());
        for (auto &&srv : tree.get_child("externalServerList"))
            if (srv.second.get_value<std::string>() != external_local_address)
                exSrvList.emplace_back(srv.second.get_value<std::string>());
        for (auto &&clt : tree.get_child("clientList")) {
            cltList.emplace_back(clt.second.get_value<std::string>());
            pImpl->clientStubs.emplace_back(external::External::NewStub((grpc::CreateChannel(
                    clt.second.get_value<std::string>(), grpc::InsecureChannelCredentials()))));
        }

        clustsize = srvList.size();
        uint32_t cnt = 0;
        for (const auto &srv : srvList) {
            pImpl->stubs.emplace_back(rpc::RaftRpc::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
            getServer[srv] = cnt++;
            nextIndex.emplace_back();
            matchIndex.emplace_back();
        }

        cnt = 0;
        for (const auto &srv : exSrvList) {
            pImpl->redirectStubs.emplace_back(external::External::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
            getExServer[srv] = cnt++;
        }

        cnt = 0;
        for (const auto &clt : cltList) {
            getClient[clt] = cnt++;
        }

        currentTerm = 0;
        lastApplied = 0;
        commitIndex = 0;
        work_is_done = false;
        log.emplace_back(0, 0, "abclzrStartKey", "abclzrStartValue");

        heart.bindElection([this](){pushElection();});
        heart.bindHeartBeat([this](){pushHearBeat();});
        if (DEBUG) std::cerr << std::setw(20) << local_address<< " : Finish Construction!" << std::endl;
    }

    void Server::RunExternal() {
        ExternalRpcService service;
        service.bindPut([this](const external::PutRequest *request, external::Reply *response) {
            put(request, response);
        });
        service.bindGet([this](const external::GetRequest *request, external::Reply *response) {
            get(request, response);
        });
        grpc::ServerBuilder builder;
        builder.AddListeningPort(external_local_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        serverExternal = builder.BuildAndStart();
        if (DEBUG) std::cout << std::setw(20) << external_local_address << " : Start External" << std::endl;
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
        if (DEBUG) std::cout << std::setw(20) << local_address << " : Start Raft" << std::endl;
        serverRaft->Wait();
    }

    void Server::RunProcess() {
        if (DEBUG) std::cout << std::setw(20) << local_address << " : Start Process" << std::endl;
        while (!work_is_done) {
            boost::unique_lock<boost::mutex> lock(mu);
            cv.wait(lock);
            while (!q.empty()) {
                event e = q.front();
                processEvent(e);
                q.pop();
            }
        }
    }

    void Server::Run() {
        st = clock();
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
        heart.Stop();
        t1.interrupt();
        t2.interrupt();
        t3.interrupt();
        if (t1.joinable()) t1.join();
        if (t2.joinable()) t2.join();
        if (t3.joinable()) t3.join();
    }

    Server::~Server() {
    }

    uint64_t Server::get_lastlogindex() {
        return (--log.end())->index;
    }

    uint64_t Server::get_lastlogterm() {
        return (--log.end())->term;
    }

    void Server::processEvent(const event &e) {
        if (DEBUG) {std::cerr << (double) (clock() - st) / 1000 << " " << std::setw(20) << local_address << " received: " << e.print()<<std::endl;}
        switch (e.type) {
            case EventType::HeartBeat:
                heartBeat();
                break;
            case EventType::RequestAppendEntries:
                AppendEntries(e.RequestAE);
                break;
            case EventType::RequestVote:
                Vote(e.RequestV);
                break;
            case EventType::Put:
                Put(e.put);
                break;
            case EventType::Get:
                Get(e.get);
                break;
            case EventType::ReplyAppendEntries:
                replyAppendEntries(e.ReplyAE);
                break;
            case EventType::ReplyVote:
                replyVote(e.ReplyV);
                break;
        }
        while (lastApplied < commitIndex) {
            ++lastApplied;
            table[log[lastApplied].key] = log[lastApplied].args;
        }
    }

    void Server::put(const external::PutRequest *request, external::Reply *response) {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
    }

    void Server::get(const external::GetRequest *request, external::Reply *response) {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
    }

    void Server::requestAE(const rpc::RequestAppendEntries *request, rpc::Reply *reply) {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
    }

    void Server::requestV(const rpc::RequestVote *request, rpc::Reply *reply) {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
    }

    void Server::replyAE(const rpc::ReplyAppendEntries *request, rpc::Reply *reply) {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
    }

    void Server::replyV(const rpc::ReplyVote *request, rpc::Reply *reply) {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
    }

    void Server::AppendEntries(const event::RequestAppendEntries *p) {
        grpc::ClientContext ctx;
        rpc::ReplyAppendEntries reply;
        rpc::Reply rp;
        auto startTimePoint = std::chrono::system_clock::now();
        ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
        ctx.set_idempotent(true);

        if (getState() != State::Follower) {
            if (getState() == State::Candidate) {
                becomeFollower();
            } else {
                if (p->term > currentTerm) {
                    currentTerm = p->term;
                    becomeFollower();
                }
            }
        } else
            becomeFollower();
        votedFor.clear();
        leaderAddress = p->leaderID;
        exLeaderAddress = p->exleaderID;
        reply.set_followerid(local_address);
        reply.set_term(currentTerm);
        if (p->term < currentTerm) reply.set_ans(false);
        else if (getTerm(p->prevLogIndex) != p->prevLogTerm) reply.set_ans(false);
        else {
            int num = log.size() - p->prevLogIndex - 1;
            for (int i = 1; i <= num; ++i) log.pop_back();
            for (const auto & i : p->entries) {
                auto ind = log.size();
                log.emplace_back(i.term, ind, i.key, i.args);
            }
            reply.set_ans(true);
        }

        if (p->term > currentTerm) currentTerm = p->term;
        if (p->leaderCommit > commitIndex) commitIndex = std::min(p->leaderCommit, log.size() - 1);
        pImpl->stubs[getServer[p->leaderID]]->ReplyAE(&ctx, reply, &rp);
    }

    void Server::Vote(const event::RequestVote *p) {
        grpc::ClientContext ctx;
        rpc::ReplyVote reply;
        rpc::Reply rp;
        auto startTimePoint = std::chrono::system_clock::now();
        ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
        ctx.set_idempotent(true);

        reply.set_followerid(local_address);
        reply.set_term(currentTerm);
        if (p->term < currentTerm) reply.set_ans(false);
        else if (p->term > currentTerm) {
            reply.set_ans(true);
            becomeFollower();
        }
        else if (!votedFor.empty()) reply.set_ans(false);
        else if (get_lastlogterm() < p->lastLogTerm ||
            (get_lastlogterm() == p->lastLogTerm && get_lastlogindex() <= p->lastLogIndex)) {
            votedFor = p->candidateID;
            reply.set_ans(true);
        } else
            reply.set_ans(false);
        if (p->term > currentTerm) currentTerm = p->term;
        grpc::Status rr;
        rr = pImpl->stubs[getServer[p->candidateID]]->ReplyV(&ctx, reply, &rp);
    }

    void Server::replyAppendEntries(const event::ReplyAppendEntries *p) {
        if (getState() == State::Leader) {
            if (p->ans) matchIndex[getServer[p->followerID]] = preMatch;
            else --nextIndex[getServer[p->followerID]];
            if (p->ans) {
                ++replyNum;
                minMatch = std::min(minMatch, matchIndex[getServer[p->followerID]]);
                if (replyNum > clustsize / 2) {
                    commitIndex = minMatch;
                    for (const auto & i : putClient) {
                        grpc::ClientContext ctx;
                        external::PutReply reply;
                        external::Reply rp;
                        auto startTimePoint = std::chrono::system_clock::now();
                        ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
                        ctx.set_idempotent(true);
                        reply.set_status(true);
                        pImpl->clientStubs[getClient[i]]->ReplyPut(&ctx, reply, &rp);
                    }
                    putClient.clear();
                }
            }
        }
    }

    void Server::replyVote(const event::ReplyVote *p) {
        if (getState() == State::Candidate) {
            if (p->ans) ++votesnum;
            if (votesnum > clustsize / 2) {
                becomeLeader();
            }
        }
    }

    void Server::Put(const event::Put *p) {
        if (getState() == State::Leader) {
            auto ind = log.size();
            log.emplace_back(LogEntry(currentTerm, ind, p->key, p->value));
            putClient.emplace_back(p->client);
        } else {
            grpc::ClientContext ctx;
            external::PutRequest request;
            external::Reply reply;
            auto startTimePoint = std::chrono::system_clock::now();
            ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
            ctx.set_idempotent(true);
            request.set_key(p->key);
            request.set_value(p->value);
            request.set_client(p->client);
            pImpl->redirectStubs[getExServer[exLeaderAddress]]->Put(&ctx, request, &reply);
        }
    }

    void Server::Get(const event::Get *p) {
        if (getState() == State::Leader) {
            grpc::ClientContext ctx;
            external::GetReply reply;
            external::Reply rp;
            auto startTimePoint = std::chrono::system_clock::now();
            ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
            ctx.set_idempotent(true);
            reply.set_status(true);
            reply.set_value(table[p->key]);
            pImpl->clientStubs[getClient[p->client]]->ReplyGet(&ctx, reply, &rp);
        } else {
            grpc::ClientContext ctx;
            external::GetRequest request;
            external::Reply reply;
            auto startTimePoint = std::chrono::system_clock::now();
            ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
            ctx.set_idempotent(true);
            request.set_key(p->key);
            request.set_client(p->client);
            pImpl->redirectStubs[getExServer[exLeaderAddress]]->Get(&ctx, request, &reply);
        }
    }

    uint64_t Server::getTerm(uint64_t index) {
        if (index >= log.size()) return 0;
        return log[index].term;
    }

    State Server::getState() {
        return heart.state;
    }

    void Server::heartBeat() {
        int tmp = 0; minMatch = preMatch = log.size() - 1; replyNum = 1;
        for (const auto & i : pImpl->stubs) {
            grpc::ClientContext ctx;
            rpc::RequestAppendEntries request;
            rpc::Reply reply;
            auto startTimePoint = std::chrono::system_clock::now();
            ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
            ctx.set_idempotent(true);
            request.set_term(currentTerm);
            request.set_leaderid(local_address);
            request.set_exleaderid(external_local_address);
            request.set_prevlogterm(getTerm(nextIndex[tmp] - 1));
            request.set_prevlogindex(nextIndex[tmp] - 1);
            for (uint32_t j = nextIndex[tmp]; j < log.size(); ++j) {
                rpc::Entry *p = request.add_entries();
                p->set_term(log[j].term);
                p->set_key(log[j].key);
                p->set_args(log[j].args);
            }
            request.set_leadercommit(commitIndex);
            i->RequestAE(&ctx, request, &reply);
            ++tmp;
        }
    }

    void Server::becomeLeader() {
        if (DEBUG2) {std::cerr << (double) (clock() - st) / 1000 << " " << std::setw(20) << local_address << " : become Leader! Term : " << currentTerm <<std::endl;}
        votedFor.clear();
        heart.becomeLeader();
        for (auto & i : nextIndex) i = log.size();
        for (auto & i : matchIndex) i = 0;
        heartBeat();
    }

    void Server::becomeFollower() {
        if (DEBUG2) {std::cerr << (double) (clock() - st) / 1000 << " " << std::setw(20) << local_address << " : become Follower! Term : " << currentTerm << std::endl;}
        votedFor.clear();
        heart.becomeFollower();
    }

    void Server::pushElection() {
        if (DEBUG2) {std::cerr << (double) (clock() - st) / 1000 << " " << std::setw(20) << local_address << " : become Candidate! Term : "<<currentTerm+1<<std::endl;}
        boost::lock_guard<boost::mutex> lock(mu);
        votesnum = 1; ++currentTerm;
        votedFor = local_address;
        for (const auto & i : pImpl->stubs) {
            grpc::ClientContext ctx;
            rpc::RequestVote request;
            rpc::Reply reply;
            auto startTimePoint = std::chrono::system_clock::now();
            ctx.set_deadline(startTimePoint + std::chrono::milliseconds(RPC_TIME_OUT));
            ctx.set_idempotent(true);
            request.set_term(currentTerm);
            request.set_candidateid(local_address);
            request.set_lastlogterm(get_lastlogterm());
            request.set_lastlogindex(get_lastlogindex());
            grpc::Status rr;
            rr = i->RequestV(&ctx, request, &reply);
        }
    }

    void Server::pushHearBeat() {
        boost::lock_guard<boost::mutex> lock(mu);
        heartBeat();
    }

    void Server::WriteLog(std::ostream &os) {
        os << local_address << " Log : contains " << log.size() << (log.size() == 1 ? " item" : " items") << " Applied " << lastApplied << " items." << std::endl;
        for (int i = 1; i <= 55; ++i) os << '-'; os << std::endl;
        os << "|index| term|        key         |        value       |" << std::endl;
        os << "|-----|-----|--------------------|--------------------|" << std::endl;
        for (int i = 0; i < log.size(); ++i) {
            std::cout << '|' << fillCenter(std::to_string(log[i].index), 5)
            << '|' << fillCenter(std::to_string(log[i].term), 5)
            << '|' << fillCenter(log[i].key, 20)
            << '|' << fillCenter(log[i].args, 20)
            << '|' << std::endl;
        }
        for (int i = 1; i <= 55; ++i) os << '-'; os << std::endl;
    }

    std::string Server::fillCenter(const std::string &s, int len) {
        if (s.size() > len) return s;
        else {
            int x1 = len - s.size();
            int x2 = x1 / 2;
            int x3 = x1 - x2;
            std::string str;
            for (int i = 1; i <= x2; ++i) str += " ";
            str += s;
            for (int i = 1; i <= x3; ++i) str += " ";
            return str;
        }
    }

    Server::LogEntry::LogEntry(uint64_t _term, uint64_t _index, const std::string &_key, const std::string &_args)
         : term(_term), index(_index), key(_key), args(_args) {}

}