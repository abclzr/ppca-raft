//
// Created by abclzr on 19-7-29.
//

#ifndef RAFT_EVENT_H
#define RAFT_EVENT_H

#include "raft/Common/raft.pb.h"
#include "raft/Common/raft.grpc.pb.h"
#include "raft/Common/external.pb.h"
#include "raft/Common/external.grpc.pb.h"
#include <cstdint>
#include <string>

namespace raft {

    enum EventType{Election, ElectionDone, RequestAppendEntries, RequestVote,
            Put, Get, ReplyAppendEntries, ReplyVote};

    class event {
    public:
        EventType type;
        struct Entry {
            uint64_t term;
            std::string key;
            std::string args;
            Entry(uint64_t, std::string, std::string);
        };

        struct RequestAppendEntries {
            uint64_t term;
            std::string leaderID;
            uint64_t prevLogIndex;
            uint64_t prevLogTerm;
            std::vector<Entry>entries;
            uint64_t leaderCommit;
            explicit RequestAppendEntries(rpc::RequestAppendEntries *);
        } *RequestAE;

        struct RequestVote {
            uint64_t term = 1;
            std::string candidateID = 2;
            int64_t lastLogIndex = 3;
            uint64_t lastLogTerm = 4;
            explicit RequestVote(rpc::RequestVote *);
        } *RequestV;

        struct Put {
            std::string key;
            std::string value;
            explicit Put(rpc::PutRequest *);
        } *put;

        struct Get {
            std::string key;
            explicit Get(rpc::GetRequest *);
        } *get;

        struct ReplyAppendEntries {
            uint64_t term;
            bool ans;
            std::string followerID;
            explicit ReplyAppendEntries(rpc::ReplyAppendEntries *);
        } *ReplyAE;

        struct ReplyVote {
            uint64_t term;
            bool ans;
            std::string followerID;
            explicit ReplyVote(rpc::ReplyVote *);
        } *ReplyV;

        explicit event(EventType);
        explicit event(rpc::RequestAppendEntries *);
        explicit event(rpc::RequestVote *);
        explicit event(rpc::PutRequest *);
        explicit event(rpc::GetRequest *);
        explicit event(rpc::ReplyAppendEntries *);
        explicit event(rpc::ReplyVote *);

    };

}

#endif //RAFT_EVENT_H
