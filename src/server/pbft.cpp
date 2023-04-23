#include "../core/logger.hpp"
#include "../core/host_manager.hpp"
#include "../core/block.hpp"
#include "blockchain.hpp"
#include "mempool.hpp"
#include "pbft.hpp"

void broadcastMessage(HostManager& hm, PBFTState type, SignedMessage msg) {
    std::vector<string> hosts = hm.getHosts(false);
    for(string host : hosts) {
        Logger::logStatus("Broadcasting new message to " + host);
        sendPBFTMessage(host, msg);
    }
}

void broadcastBlock(HostManager& hm, Block block) {
    std::vector<string> hosts = hm.getHosts(false);
    for(string host : hosts) {
        Logger::logStatus("Broadcasting new block to " + host);
        sendBlockProposal(host, block);
    }
}

SignedMessage createPBFTMessage(SHA256Hash hash, PublicKey pub, PrivateKey priv, PBFTState type) {
    SignedMessage msg;
    msg.hash = hash;
    msg.publicKey = pub;
    msg.signature = signWithPrivateKey(SHA256toString(hash), pub, priv);
    msg.type = type;

    return msg;
}

PBFTManager::PBFTManager(HostManager& h, BlockChain& b, MemPool& m) : hosts(h), blockchain(b), mempool(m) {
    Logger::logStatus("PBFTManager initialized!");
    user = User();
    state = IDLE;
}

// void PBFTManager::proposeBlock() {
//     if(state != IDLE || !isProposer()) return;
//     state = PROPOSING;

//     Logger::logStatus("Block proposal called!");

//     Block block = createBlock();
//     blockPool.insert(block);
//     broadcastBlock(hosts, block);
// };

void PBFTManager::prePrepare(Block& block) {
    if(blockPool.find(block) != blockPool.end())
        return;

    Logger::logStatus("PrePrepare called!");

    ExecutionStatus isValid = blockchain.validateBlock(block);
    Logger::logStatus("Validity of the block: " + executionStatusAsString(isValid));

    if(isValid != SUCCESS)
        return;

    blockPool.insert(block);
    broadcastBlock(hosts, block);

    SignedMessage msg = createPBFTMessage(block.getHash(), user.getPublicKey(), user.getPrivateKey(), PREPARING);
    preparePool.insert(msg);
    broadcastMessage(hosts, PREPARING, msg);
};

void PBFTManager::prepare(SignedMessage msg) {
    Logger::logStatus("Prepare called!");
    // check validity of the prepare message
    // if valid add message to the prepare pool
    // broadcast the prepare message
    // if the prepare pool is bigger than min approvals 
    // then add data to commit pool and broadcast commit
}

void PBFTManager::commit(SignedMessage msg) {
    Logger::logStatus("Commit called!");
    // check validity of the commit message
    // if valid add message to the commitpool
    // broadcast the commit message
    // if commit pool is bigger than min approvals
    // then commit the block to the chain and broadcast round change
}

// do we need this step?
void PBFTManager::roundChange(SignedMessage msg) {
    Logger::logStatus("Round change called!");
    // check validity of the round change message
    // add to message pool and broadcast
    // if message pool is bigger min approvals
    // then clear the transaction queue
}

// bool PBFTManager::isProposer() {
//     return hosts.getAddress() == "http://localhost:3000";
// }