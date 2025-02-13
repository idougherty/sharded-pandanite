#pragma once
#include <vector>
#include <map>
#include "transaction.hpp"
#include "constants.hpp"
#include "common.hpp"
#include "crypto.hpp"
#include "helpers.hpp"
#include "merkle_tree.hpp"
using namespace std;

struct BlockHeader {
    uint32_t id;
    uint64_t timestamp; 
    uint32_t difficulty;
    uint32_t numTransactions;
    SHA256Hash lastBlockHash;
    SHA256Hash merkleRoot;
    // SHA256Hash nonce;
    array<SignedMessage, MIN_APPROVALS> signatures;
};

// #define BLOCKHEADER_BUFFER_SIZE 116
#define BLOCKHEADER_BUFFER_SIZE 544

BlockHeader blockHeaderFromBuffer(const char* buffer);
void blockHeaderToBuffer(BlockHeader& t, char* buffer);

class Block {
    public:
        Block();
        Block(json data);
        Block(const Block& b);
        Block(const BlockHeader&b, vector<Transaction>& transactions);
        BlockHeader serialize();
        json toJson();
        void addTransaction(Transaction t);
        void setNonce(SHA256Hash s);
        void setMerkleRoot(SHA256Hash s);
        void setTimestamp(uint64_t t);
        void setId(uint32_t id);
        void setDifficulty(uint8_t d);
        SHA256Hash getHash() const;
        // SHA256Hash getNonce() const;
        SHA256Hash getMerkleRoot() const;
        SHA256Hash getLastBlockHash() const;
        void setLastBlockHash(SHA256Hash hash);
        uint64_t getTimestamp() const;
        uint32_t getDifficulty() const;
        const vector<Transaction>& getTransactions() const;
        vector<Transaction>& getTransactions();
        uint32_t getId() const;
        // bool verifyNonce();
        bool verifySignatures();
    // protected:
        uint32_t id;
        uint64_t timestamp;
        uint32_t difficulty;
        vector<Transaction> transactions;
        SHA256Hash merkleRoot;
        SHA256Hash lastBlockHash;
        // SHA256Hash nonce;
        array<SignedMessage, MIN_APPROVALS> signatures;
    private:
        friend bool operator==(const Block& a, const Block& b);
};

bool operator==(const Block& a, const Block& b);
