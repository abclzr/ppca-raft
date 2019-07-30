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

    Server::Server(const std::string &filename, uint64_t _clustnum, const std::string &Clientaddr) : pImpl(std::make_unique<Impl>()) {
        namespace pt = boost::property_tree;
        pt::ptree tree;
        pt::read_json(filename, tree);

        for (auto &&srv : tree.get_child("local"))
            local_address = srv.second.get_value<std::string>();
        std::vector<std::string> srvList;
        for (auto &&srv : tree.get_child("serverList"))
            if (srv.second.get_value<std::string>() != local_address)
                srvList.emplace_back(srv.second.get_value<std::string>());

        uint32_t cnt = 0;
        for (const auto &srv : srvList) {
            pImpl->redirectStubs.emplace_back(external::External::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
            pImpl->stubs.emplace_back(rpc::RaftRpc::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
            getServer[srv] = cnt++;
        }

        if (!Clientaddr.empty())
            pImpl->clientStubs.emplace_back(external::External::NewStub((grpc::CreateChannel(
                    Clientaddr, grpc::InsecureChannelCredentials()))));

        clustsize = _clustnum;
        currentTerm = 0;
        lastApplied = 0;
        commitIndex = 0;
        work_is_done = false;
        log.emplace_back(0, 0, "", "");

        heart.bindElection([this](){pushElection();});
        heart.bindElectionDone([this](){pushElectionDone();});
        heart.bindHeartBeat([this](){pushHearBeat();});
        heart.Run();
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
        builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        serverExternal = builder.BuildAndStart();
        if (DEBUG) std::cout << std::setw(20) << local_address << " : Start External" << std::endl;
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

    void Server::RunProcess() {//TODO: unique_lock and conditional virables
        if (DEBUG) std::cout << std::setw(20) << local_address << " : Start Process" << std::endl;
        while (!work_is_done) {
            boost::unique_lock<boost::mutex> lock(mu, boost::defer_lock);
            cv.wait(lock, [this]{return !q.empty();});
            while (!q.empty()) {
                processEvent(q.front());
                q.pop();
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
        cv.notify_one();
        if (t1.joinable()) t1.join();
        if (t2.joinable()) t2.join();
        if (t3.joinable()) t3.join();
        heart.Stop();
        mu.unlock();
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
        if (DEBUG) {std::cerr << std::setw(20) << local_address << " received: " << e.print()<<std::endl;}
        switch (e.type) {
            case EventType::Election:
                election();
                break;
            case EventType::ElectionDone:
                electionDone();
                break;
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
        if (DEBUG) {std::cerr << std::setw(20) << local_address << " : rpc received requestVote" <<std::endl;}
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(request));
        cv.notify_one();
        if (DEBUG) {std::cerr << std::setw(20) << local_address << " : add requestVote to queue!" <<std::endl;}
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

        if (getState() != State::Follower) {
            if (getState() == State::Candidate) {
                becomeFollower();
            } else {
                if (p->term > currentTerm) {
                    currentTerm = p->term;
                    becomeFollower();
                }
            }
        }
        votedFor.clear();
        leaderAddress = p->leaderID;
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

        if (p->leaderCommit > commitIndex) commitIndex = std::min(p->leaderCommit, log.size() - 1);

        pImpl->stubs[getServer[p->leaderID]]->ReplyAE(&ctx, reply, &rp);
    }

    void Server::Vote(const event::RequestVote *p) {
        grpc::ClientContext ctx;
        rpc::ReplyVote reply;
        rpc::Reply rp;

        if (getState() == State::Leader && p->term > currentTerm) {
            currentTerm = p->term;
            becomeFollower();
            return;
        }
        reply.set_followerid(local_address);
        reply.set_term(currentTerm);
        if (p->term < currentTerm) reply.set_ans(false);
        else if (!votedFor.empty()) reply.set_ans(false);
        else if (get_lastlogterm() < p->lastLogTerm ||
            (get_lastlogterm() == p->lastLogTerm && get_lastlogindex() < p->lastLogIndex)) {
            votedFor = p->candidateID;
            reply.set_ans(true);
        } else
            reply.set_ans(false);

        pImpl->stubs[getServer[p->candidateID]]->ReplyV(&ctx, reply, &rp);
    }

    void Server::replyAppendEntries(const event::ReplyAppendEntries *p) {
        if (p->ans) matchIndex[getServer[p->followerID]] = log.size() - 1;
        else --nextIndex[getServer[p->followerID]];
    }

    void Server::replyVote(const event::ReplyVote *p) {
        if (p->ans) ++votesnum;
    }

    void Server::Put(const event::Put *p) {
        if (getState() == State::Leader) {
            auto ind = log.size();
            log.emplace_back(LogEntry(currentTerm, ind, p->key, p->value));
            grpc::ClientContext ctx;
            external::PutReply reply;
            external::Reply rp;
            reply.set_status(true);
            pImpl->clientStubs[0]->ReplyPut(&ctx, reply, &rp);
        } else {
            grpc::ClientContext ctx;
            external::PutRequest request;
            external::Reply reply;
            pImpl->redirectStubs[getServer[leaderAddress]]->Put(&ctx, request, &reply);
        }
    }

    void Server::Get(const event::Get *p) {
        if (getState() == State::Leader) {
            grpc::ClientContext ctx;
            external::GetReply reply;
            external::Reply rp;
            reply.set_status(true);
            reply.set_value(table[p->key]);
            pImpl->clientStubs[0]->ReplyGet(&ctx, reply, &rp);
        } else {
            grpc::ClientContext ctx;
            external::GetRequest request;
            external::Reply reply;
            pImpl->redirectStubs[getServer[leaderAddress]]->Get(&ctx, request, &reply);
        }
    }

    uint64_t Server::getTerm(uint64_t index) {
        if (index >= log.size()) return 0;
        return log[index].term;
    }

    State Server::getState() {
        return heart.state;
    }

    void Server::election() {
        becomeCandidate();
        votesnum = 0; ++currentTerm;
        for (const auto & i : pImpl->stubs) {
            grpc::ClientContext ctx;
            rpc::RequestVote request;
            rpc::Reply reply;
            request.set_term(currentTerm);
            request.set_candidateid(local_address);
            request.set_lastlogterm(get_lastlogterm());
            request.set_lastlogindex(get_lastlogindex());
            i->RequestV(&ctx, request, &reply);
            if (DEBUG) {std::cerr << std::setw(20) << local_address << " : sendVoteRequest!"<<std::endl;}
        }
    }

    void Server::electionDone() {
        if (votesnum > clustsize / 2) becomeLeader();
        else becomeFollower();
    }

    void Server::heartBeat() {
        int tmp = 0;
        for (const auto & i : pImpl->stubs) {
            grpc::ClientContext ctx;
            rpc::RequestAppendEntries request;
            rpc::Reply reply;
            request.set_term(currentTerm);
            request.set_leaderid(local_address);
            request.set_prevlogterm(get_lastlogterm());
            request.set_prevlogindex(get_lastlogindex());
            for (uint32_t j = nextIndex[tmp]; j < log.size(); ++j) {
                rpc::Entry *p = request.add_entries();
                p->set_term(log[j].term);
                p->set_key(log[j].key);
                p->set_args(log[j].args);
            }
            request.set_leadercommit(commitIndex);
            i->RequestAE(&ctx, request, &reply);
            if (DEBUG) {std::cerr << std::setw(20) << local_address << " : sendAppendEntriesRequest!"<<std::endl;}
        }
    }

    void Server::becomeLeader() {
        heart.becomeLeader();
        for (auto & i : nextIndex) i = log.size();
        for (auto & i : matchIndex) i = 0;
        heartBeat();
    }

    void Server::becomeFollower() {
        heart.becomeFollower();
    }

    void Server::becomeCandidate() {
        heart.becomeCandidate();
    }

    void Server::pushElection() {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(EventType::Election));
        cv.notify_one();
    }

    void Server::pushElectionDone() {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(EventType::ElectionDone));
        cv.notify_one();
    }

    void Server::pushHearBeat() {
        boost::lock_guard<boost::mutex> lock(mu);
        q.push(event(EventType::HeartBeat));
        cv.notify_one();
    }

    Server::LogEntry::LogEntry(uint64_t _term, uint64_t _index, const std::string &_key, const std::string &_args)
         : term(_term), index(_index), key(_key), args(_args) {}

}