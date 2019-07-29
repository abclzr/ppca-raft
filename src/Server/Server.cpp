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

    Server::Server(const std::string &filename, uint64_t _clustnum, const std::string &Clientaddr) : clustsize(_clustnum) {
        namespace pt = boost::property_tree;
        pt::ptree tree;
        pt::read_json(filename, tree);

        local_address = tree.get<std::string>("local");
        std::vector<std::string> srvList;
        for (auto &&srv : tree.get_child("serverList"))
            if (srv.second.get_value<std::string>() != local_address)
                srvList.emplace_back(srv.second.get_value<std::string>());

        uint32_t cnt = 0;
        for (const auto &srv : srvList) {
            pImpl->stubs.emplace_back(rpc::RaftRpc::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
            pImpl->redirectStubs.emplace_back(external::External::NewStub(grpc::CreateChannel(
                    srv, grpc::InsecureChannelCredentials())));
            getServer[srv] = cnt++;
        }

        pImpl->clientStubs.emplace_back(external::External::NewStub((grpc::CreateChannel(
                    Clientaddr, grpc::InsecureChannelCredentials()))));

        currentTerm = 0;
        lastApplied = 0;
        commitIndex = 0;
        work_is_done = false;
        log.emplace_back(0, 0, "", "");

        //TODO:: add heart-bind
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
                processEvent(q.front());
                q.pop();
            }
            mu.unlock();
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

    void Server::processEvent(const event &e) {
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

        votedFor.clear();
        leaderAddress = p->leaderID;
        reply.set_followerid(local_address);
        reply.set_term(currentTerm);
        if (p->term < currentTerm) reply.set_ans(false);
        else if (getTerm(p->prevLogIndex) != p->prevLogTerm) reply.set_ans(false);
        else {
            int num = log.size() - p->prevLogIndex - 1;
            for (int i = 1; i <= num; ++i) log.pop_back();
            for (const auto & i : p->entries)
                log.emplace_back(i.term, log.size(), i.key, i.args);
            reply.set_ans(true);
        }

        if (p->leaderCommit > commitIndex) commitIndex = std::min(p->leaderCommit, log.size() - 1);

        pImpl->stubs[getServer[p->leaderID]]->ReplyAE(&ctx, reply, &rp);
    }

    void Server::Vote(const event::RequestVote *p) {
        grpc::ClientContext ctx;
        rpc::ReplyVote reply;
        rpc::Reply rp;

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
    }

    void Server::replyVote(const event::ReplyVote *p) {
        if (p->ans) ++votesnum;
    }

    void Server::Put(const event::Put *p) {
        if (getState() == State::Leader) {
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
        votesnum = 0;
        for (const auto & i : pImpl->stubs) {
            grpc::ClientContext ctx;
            rpc::RequestVote request;
            rpc::Reply reply;
            request.set_term(currentTerm);
            request.set_candidateid(local_address);
            request.set_lastlogterm(get_lastlogterm());
            request.set_lastlogindex(get_lastlogindex());
            i->RequestV(&ctx, request, &reply);
        }
    }

    void Server::electionDone() {
        heart.electionResult(votesnum > clustsize / 2);
    }

    void Server::heartBeat() {
        for (const auto & i : pImpl->stubs) {

        }
    }


    Server::LogEntry::LogEntry(uint64_t _term, uint64_t _index, const std::string &_key, const std::string &_args)
         : term(_term), index(_index), key(_key), args(_args) {}

}