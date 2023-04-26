#include "block.hpp"
#include "helpers.hpp"
#include "crypto.hpp"
#include "openssl/sha.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <cstring>
using namespace std;

BlockHeader blockHeaderFromBuffer(const char* buffer) {
    BlockHeader b;
    b.id = readNetworkUint32(buffer);
    b.timestamp = readNetworkUint64(buffer);
    b.difficulty = readNetworkUint32(buffer);
    b.numTransactions = readNetworkUint32(buffer);
    b.lastBlockHash = readNetworkSHA256(buffer);
    b.merkleRoot = readNetworkSHA256(buffer);
    for(int i = 0; i < MIN_APPROVALS; i++) {
        b.signatures[i] = signedMessageFromBuffer(buffer);
    }
    // b.nonce = readNetworkSHA256(buffer);
    return b;
}

void blockHeaderToBuffer(BlockHeader& b, char* buffer) {
    writeNetworkUint32(buffer, b.id);
    writeNetworkUint64(buffer, b.timestamp);
    writeNetworkUint32(buffer, b.difficulty);
    writeNetworkUint32(buffer, b.numTransactions);
    writeNetworkSHA256(buffer, b.lastBlockHash);
    writeNetworkSHA256(buffer, b.merkleRoot);
    for(int i = 0; i < MIN_APPROVALS; i++) {
        signedMessageToBuffer(b.signatures[i], buffer);
    }
    // writeNetworkSHA256(buffer, b.nonce);
}


Block::Block() {
    // this->nonce = NULL_SHA256_HASH;
    this->id = 1;
    this->timestamp = getCurrentTime();
    this->difficulty = MIN_DIFFICULTY;
    this->merkleRoot = NULL_SHA256_HASH;
    this->lastBlockHash = NULL_SHA256_HASH;
}

Block::Block(const Block& b) {
    // this->nonce = b.nonce;
    this->id = b.id;
    this->difficulty = b.difficulty;
    this->timestamp = b.timestamp;
    this->merkleRoot = b.merkleRoot;
    this->lastBlockHash = b.lastBlockHash;
    this->transactions = vector<Transaction>();
    for(auto t : b.transactions) {
        this->transactions.push_back(t);
    }
}

Block::Block(json block) {
    // this->nonce = stringToSHA256(block["nonce"]);
    this->merkleRoot = stringToSHA256(block["merkleRoot"]);
    this->lastBlockHash = stringToSHA256(block["lastBlockHash"]);
    this->id = block["id"];
    this->difficulty = block["difficulty"];
    this->timestamp = stringToUint64(block["timestamp"]);
    for(auto t : block["transactions"]) {
        Transaction curr = Transaction(t);
        this->transactions.push_back(curr);
    }
}

Block::Block(const BlockHeader&b, vector<Transaction>& transactions) {
    this->id = b.id;
    this->timestamp = b.timestamp;
    this->difficulty = b.difficulty;
    // this->nonce= b.nonce;
    this->signatures = b.signatures;
    this->merkleRoot = b.merkleRoot;
    this->lastBlockHash = b.lastBlockHash;
    for(auto t : transactions) {
        this->addTransaction(t);
    }
}

BlockHeader Block::serialize() {
    BlockHeader b;
    b.id = this->id;
    b.timestamp = this->timestamp;
    b.difficulty = this->difficulty;
    b.numTransactions = this->transactions.size();
    // b.nonce = this->nonce;
    b.signatures = this->signatures;
    b.merkleRoot = this->merkleRoot;
    b.lastBlockHash = this->lastBlockHash;
    return b;
}

json Block::toJson() {
    json result;
    result["id"] = this->id;
    result["hash"] = SHA256toString(this->getHash());
    result["difficulty"] = this->difficulty;
    // result["nonce"] = SHA256toString(this->nonce);
    result["signatures"] = json::array();
    result["timestamp"] = uint64ToString(this->timestamp);
    result["transactions"] = json::array();
    result["merkleRoot"] = SHA256toString(this->merkleRoot);
    result["lastBlockHash"] = SHA256toString(this->lastBlockHash);
    
    for(auto t : this->transactions) {
        result["transactions"].push_back(t.toJson());
    }
    for(auto s : this->signatures) {
        json obj;
        obj["hash"] = s.hash;
        obj["publicKey"] = s.publicKey;
        obj["signature"] = s.signature;
        obj["type"] = s.type;
        result["signatures"].push_back(s.hash);
    }
    return result;
}


void Block::setTimestamp(uint64_t t) {
    this->timestamp = t;
}

uint64_t Block::getTimestamp() const{
    return this->timestamp;
}

uint32_t Block::getId() const{
    return this->id;
}

void Block::setId(uint32_t id) {
    this->id = id;
}


void Block::addTransaction(Transaction t) {
    this->transactions.push_back(t);
}

// void Block::setNonce(SHA256Hash s) {
//     this->nonce = s;
// }

// SHA256Hash Block::getNonce() const{
//     return this->nonce;
// }

void Block::setMerkleRoot(SHA256Hash s) {
    this->merkleRoot = s;
}

SHA256Hash Block::getMerkleRoot() const{
    return this->merkleRoot;
}

const vector<Transaction>& Block::getTransactions() const{
    return this->transactions;
}

vector<Transaction>& Block::getTransactions(){
    return this->transactions;
}

// bool Block::verifyNonce() {
//     SHA256Hash target = this->getHash();
//     bool usePufferfish = this->getId() > PUFFERFISH_START_BLOCK;
//     return verifyHash(target, this->nonce, this->difficulty, usePufferfish, true);
// }
bool Block::verifySignatures() {
    for(SignedMessage s : this->signatures) {
        string targetStr = SHA256toString(this->getHash()) + s.address + publicKeyToString(s.publicKey);
        SHA256Hash target = SHA256(targetStr);
        if(!verifyHash(target, s.solution, this->difficulty, false, true));
            false;
    }
    return true;
}

void Block::setDifficulty(uint8_t d) {
    this->difficulty = d;
}

uint32_t Block::getDifficulty() const {
    return this->difficulty;
}

SHA256Hash Block::getLastBlockHash() const{
    return this->lastBlockHash;
}

void Block::setLastBlockHash(SHA256Hash hash) {
    this->lastBlockHash = hash;
}

SHA256Hash Block::getHash() const{
    SHA256Hash ret;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, (unsigned char*)this->merkleRoot.data(), this->merkleRoot.size());
    SHA256_Update(&sha256, (unsigned char*)this->lastBlockHash.data(), this->lastBlockHash.size());
    SHA256_Update(&sha256, (unsigned char*)&this->difficulty, sizeof(uint32_t));
    SHA256_Update(&sha256, (unsigned char*)&this->timestamp, sizeof(uint64_t));
    SHA256_Final(ret.data(), &sha256);
    return ret;
}

bool operator==(const Block& a, const Block& b) {
    if(b.id != a.id) return false;
    // if(b.nonce != a.nonce) return false;)
    if(b.timestamp != a.timestamp) return false;
    if(b.lastBlockHash != a.lastBlockHash) return false;
    if(b.difficulty != a.difficulty) return false;
    if(b.merkleRoot != a.merkleRoot) return false;
    // check transactions equal
    if (a.transactions.size() != b.transactions.size()) return false;
    for(int i =0; i < a.transactions.size(); i++) {
        if(a.transactions[i] == b.transactions[i]) {
            continue;
        } else {
            return false;
        }
    }

    for(int i = 0; i < MIN_APPROVALS; i++) {
        if(a.signatures[i].signature != b.signatures[i].signature) {
            return false;
        }
    }
    return true;
}
