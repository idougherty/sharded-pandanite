#include "../core/logger.hpp"
#include "../core/host_manager.hpp"
#include "../core/block.hpp"
#include "blockchain.hpp"
#include "mempool.hpp"
#include "pbft.hpp"

void broadcastMessage(HostManager& hm, SignedMessage msg) {
    std::vector<string> hosts = hm.getHosts(false);
    for(string host : hosts) {
        Logger::logStatus("Broadcasting new message to " + host);
        
        // Dispatching thread to prevent blocking
        std::thread([host, msg]() {
            sendPBFTMessage(host, msg);
        }).detach();
    }
}

void broadcastBlock(HostManager& hm, Block& block) {
    std::vector<string> hosts = hm.getHosts(false);
    for(string host : hosts) {
        Logger::logStatus("Broadcasting new block to " + host);

        // Dispatching thread to prevent blocking
        std::thread([host, block]() {
            try {
                Block a = block;
                sendBlockProposal(host, a);
            } catch(...) {
                Logger::logStatus("Could not forward new block to " + host);
            }
        }).detach();
    }
}

SignedMessage createPBFTMessage(SHA256Hash hash, PublicKey pub, PrivateKey priv, PBFTState type) {
    SignedMessage msg;
    msg.hash = hash;
    msg.publicKey = pub;
    msg.signature = signWithPrivateKey((const char*)hash.data(), hash.size(), pub, priv);
    msg.type = type;

    return msg;
}

PBFTManager::PBFTManager(HostManager& h, BlockChain& b, MemPool& m) : hosts(h), blockchain(b), mempool(m) {
    Logger::logStatus("PBFTManager initialized!");
    user = User();
    state = IDLE;
}

void PBFTManager::prePrepare(Block& block) {
    if(blockPool.find(block.getHash()) != blockPool.end())
        return;

    Logger::logStatus("PrePrepare called!");

    //TODO: doesn't validate transaction validity yet
    if(blockchain.validateBlock(block) != SUCCESS)
        return;

    blockPool.insert({ block.getHash(), block});
    broadcastBlock(hosts, block);

    SignedMessage msg = createPBFTMessage(block.getHash(), user.getPublicKey(), user.getPrivateKey(), PREPARING);
    insertIntoPool(preparePool, msg);
    broadcastMessage(hosts, msg);
};

void PBFTManager::prepare(SignedMessage msg) {
    if(poolHasMessage(preparePool, msg))
        return;

    Logger::logStatus("Prepare called!");

    bool isValid = checkSignature((const char*)msg.hash.data(), msg.hash.size(), msg.signature, msg.publicKey);
    if(!isValid)
        return;

    insertIntoPool(preparePool, msg);
    broadcastMessage(hosts, msg);

    if(messagePoolSize(preparePool, msg) < MIN_APPROVALS)
        return;

    SignedMessage commitMsg = createPBFTMessage(msg.hash, user.getPublicKey(), user.getPrivateKey(), COMMITTING);
    insertIntoPool(commitPool, commitMsg);
    broadcastMessage(hosts, commitMsg);
}

void PBFTManager::commit(SignedMessage msg) {
    if(poolHasMessage(commitPool, msg))
        return;

    Logger::logStatus("Commit called!");

    bool isValid = checkSignature((const char*)msg.hash.data(), msg.hash.size(), msg.signature, msg.publicKey);
    if(!isValid)
        return;

    insertIntoPool(commitPool, msg);
    broadcastMessage(hosts, msg);

    if(messagePoolSize(commitPool, msg) >= MIN_APPROVALS) {
        Block block = blockPool.at(msg.hash);
        ExecutionStatus status = blockchain.addBlock(block);
    }

    SignedMessage roundChangeMsg = createPBFTMessage(msg.hash, user.getPublicKey(), user.getPrivateKey(), ROUND_CHANGE);
    insertIntoPool(commitPool, roundChangeMsg);
    broadcastMessage(hosts, roundChangeMsg);
}

// do we need this step?
void PBFTManager::roundChange(SignedMessage msg) {
    Logger::logStatus("Round change called!");
    // check validity of the round change message
    // add to message pool and broadcast
    // if message pool is bigger min approvals
    // then clear the transaction queue
}

bool PBFTManager::poolHasMessage(MessagePool& pool, SignedMessage msg) {
    auto pair = pool.find(msg.hash);
    if(pair == pool.end())
        return false;
    return pair->second.find(msg) != pair->second.end();
}

void PBFTManager::insertIntoPool(MessagePool& pool, SignedMessage msg) {
    auto pair = pool.find(msg.hash);
    if(pair == pool.end()) {
        unordered_set<SignedMessage, MessageHash> msgSet;
        msgSet.insert(msg);
        pool.insert({ msg.hash, msgSet });
    } else {
        pair->second.insert(msg);
    }
}

size_t PBFTManager::messagePoolSize(MessagePool& pool, SignedMessage msg) {
    auto pair = pool.find(msg.hash);
    if(pair == pool.end())
        return 0;
    return pair->second.size();
}