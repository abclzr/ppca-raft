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

    enum EventType{Election, ElectionDone, HeartBeat, RequestAppendEntries, RequestVote,
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
            explicit RequestAppendEntries(const rpc::RequestAppendEntries *);
        } *RequestAE;

        struct RequestVote {
            uint64_t term = 1;
            std::string candidateID = 2;
            int64_t lastLogIndex = 3;
            uint64_t lastLogTerm = 4;
            explicit RequestVote(const rpc::RequestVote *);
        } *RequestV;

        struct Put {
            std::string key;
            std::string value;
            explicit Put(const external::PutRequest *);
        } *put;

        struct Get {
            std::string key;
            explicit Get(const external::GetRequest *);
        } *get;

        struct ReplyAppendEntries {
            uint64_t term;
            bool ans;
            std::string followerID;
            explicit ReplyAppendEntries(const rpc::ReplyAppendEntries *);
        } *ReplyAE;

        struct ReplyVote {
            uint64_t term;
            bool ans;
            std::string followerID;
            explicit ReplyVote(const rpc::ReplyVote *);
        } *ReplyV;

        explicit event(EventType);
        explicit event(const rpc::RequestAppendEntries *);
        explicit event(const rpc::RequestVote *);
        explicit event(const external::PutRequest *);
        explicit event(const external::GetRequest *);
        explicit event(const rpc::ReplyAppendEntries *);
        explicit event(const rpc::ReplyVote *);

    };

}

#endif //RAFT_EVENT_H
