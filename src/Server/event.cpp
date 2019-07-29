//
// Created by abclzr on 19-7-29.
//

#include <raft/Server/event.h>

raft::event::Entry::Entry(uint64_t t, std::string k, std::string a)
 : term(t), key(k), args(a) {}

raft::event::RequestAppendEntries::RequestAppendEntries(raft::rpc::RequestAppendEntries * p) {
    term = p->term();
    leaderID = p->leaderid();
    prevLogIndex = p->prevlogindex();
    prevLogTerm = p->prevlogterm();
    for (const auto & i : p->entries())
        entries.emplace_back(Entry(i.term(), i.key(), i.args()));
    leaderCommit = p->leadercommit();
}

raft::event::RequestVote::RequestVote(raft::rpc::RequestVote *p) {
    term = p->term();
    candidateID = p->candidateid();
    lastLogIndex = p->lastlogindex();
    lastLogTerm = p->lastlogterm();
}

raft::event::Put::Put(raft::rpc::PutRequest *p) {
    key = p->key();
    value = p->value();
}

raft::event::Get::Get(raft::rpc::GetRequest *p) {
    key = p->key();
}

raft::event::ReplyAppendEntries::ReplyAppendEntries(raft::rpc::ReplyAppendEntries *p) {
    term = p->term();
    ans = p->ans();
    followerID = p->followerid();
}

raft::event::ReplyVote::ReplyVote(raft::rpc::ReplyVote *p) {
    term = p->term();
    ans = p->ans();
    followerID = p->followerid();
}

raft::event::event(raft::EventType t) : type(t) {
    switch (type) {
        case EventType::Election:
            break;
        case EventType::ElectionDone:
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

raft::event::event(raft::rpc::RequestAppendEntries *p) {
    RequestAE = new RequestAppendEntries(p);
    type = EventType::RequestAppendEntries;
}

raft::event::event(raft::rpc::RequestVote *p) {
    RequestV = new RequestVote(p);
    type = EventType::RequestVote;
}

raft::event::event(raft::rpc::PutRequest *p) {
    put = new Put(p);
    type = EventType::Put;
}

raft::event::event(raft::rpc::GetRequest *p) {
    get = new Get(p);
    type = EventType::Get;
}

raft::event::event(raft::rpc::ReplyAppendEntries *p) {
    ReplyAE = new ReplyAppendEntries(p);
    type = EventType::ReplyAppendEntries;
}

raft::event::event(raft::rpc::ReplyVote *p) {
    ReplyV = new ReplyVote(p);
    type = EventType::ReplyVote;
}
