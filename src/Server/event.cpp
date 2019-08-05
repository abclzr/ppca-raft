//
// Created by abclzr on 19-7-29.
//

#include <raft/Server/event.h>

raft::event::Entry::Entry(uint64_t t, std::string k, std::string a)
 : term(t), key(k), args(a) {}

raft::event::RequestAppendEntries::RequestAppendEntries(const raft::rpc::RequestAppendEntries * p) {
    term = p->term();
    leaderID = p->leaderid();
    prevLogIndex = p->prevlogindex();
    prevLogTerm = p->prevlogterm();
    for (const auto & i : p->entries())
        entries.emplace_back(Entry(i.term(), i.key(), i.args()));
    leaderCommit = p->leadercommit();
    exleaderID = p->exleaderid();
}

raft::event::RequestVote::RequestVote(const raft::rpc::RequestVote *p) {
    term = p->term();
    candidateID = p->candidateid();
    lastLogIndex = p->lastlogindex();
    lastLogTerm = p->lastlogterm();
}

raft::event::Put::Put(const raft::external::PutRequest *p) {
    key = p->key();
    value = p->value();
    client = p->client();
}

raft::event::Get::Get(const raft::external::GetRequest *p) {
    key = p->key();
    client = p->client();
}

raft::event::ReplyAppendEntries::ReplyAppendEntries(const raft::rpc::ReplyAppendEntries *p) {
    term = p->term();
    ans = p->ans();
    followerID = p->followerid();
}

raft::event::ReplyVote::ReplyVote(const raft::rpc::ReplyVote *p) {
    term = p->term();
    ans = p->ans();
    followerID = p->followerid();
}

raft::event::event(raft::EventType t) : type(t) {
    switch (type) {
        case EventType::HeartBeat:
            break;
        case EventType::RequestAppendEntries:
            break;
        case EventType::RequestVote:
            break;
        case EventType::Put:
            break;
        case EventType::Get:
            break;
        case EventType::ReplyAppendEntries:
            break;
        case EventType::ReplyVote:
            break;
    }
}

raft::event::event(const raft::rpc::RequestAppendEntries *p) {
    RequestAE = new RequestAppendEntries(p);
    type = EventType::RequestAppendEntries;
}

raft::event::event(const raft::rpc::RequestVote *p) {
    RequestV = new RequestVote(p);
    type = EventType::RequestVote;
}

raft::event::event(const raft::external::PutRequest *p) {
    put = new Put(p);
    type = EventType::Put;
}

raft::event::event(const raft::external::GetRequest *p) {
    get = new Get(p);
    type = EventType::Get;
}

raft::event::event(const raft::rpc::ReplyAppendEntries *p) {
    ReplyAE = new ReplyAppendEntries(p);
    type = EventType::ReplyAppendEntries;
}

raft::event::event(const raft::rpc::ReplyVote *p) {
    ReplyV = new ReplyVote(p);
    type = EventType::ReplyVote;
}

std::string raft::event::print() const {
    std::string ans;
    ans.clear();

    switch (type) {
        case EventType::HeartBeat:
            ans += "HeartBeat: ";
            break;
        case EventType::RequestAppendEntries:
            ans += "RequestAppendEntries: ";
            ans += RequestAE->leaderID;
            break;
        case EventType::RequestVote:
            ans += "RequestVote: ";
            ans += RequestV->candidateID;
            break;
        case EventType::Put:
            ans += "Put: ";
            break;
        case EventType::Get:
            ans += "Get: ";
            break;
        case EventType::ReplyAppendEntries:
            ans += "ReplyAppendEntries: ";
            ans += ReplyAE->followerID;
            break;
        case EventType::ReplyVote:
            ans += "ReplyVote: ";
            ans += ReplyV->followerID;
            break;
    }
    return ans;
}
