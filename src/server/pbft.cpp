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

//TODO: multicast to directory
void broadcastSignedBlock(HostManager& hm, Block block, SignedMessage signature[]) {
    std::vector<string> hosts = hm.getHosts(false);
    for(string host : hosts) {
        Logger::logStatus("Broadcasting signed blocks to to " + host);

        std::thread([host, block, signature]() {
            try {
                sendSignedBlock(host, block, signature);
            } catch(...) {
                Logger::logStatus("Could not forward signed blocks to " + host);
            }
        }).detach();
    }
}

//TODO: multicast to directory
void broadcastSignedBlocks(HostManager& hm, vector<Block> blocks, vector<array<SignedMessage, MIN_APPROVALS>> signatures) {
    std::vector<string> hosts = hm.getHosts(false);
    for(string host : hosts) {
        Logger::logStatus("Broadcasting signed blocks to to " + host);

        std::thread([host, blocks, signatures]() {
            try {
                sendSignedBlocks(host, blocks, signatures);
            } catch(...) {
                Logger::logStatus("Could not forward signed blocks to " + host);
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
    this->digest = NULL_SHA256_HASH;

    //TODO: use PoW defined identities
    user = User();
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

// TODO: implement identity checking
void PBFTManager::prePrepare(Block& block) {
    if(blockPool.find(block.getHash()) != blockPool.end())
        return;

    Logger::logStatus("PrePrepare called!");

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
        array<SignedMessage, MIN_APPROVALS> messages;
        
        int i = 0;
        auto msgSet = commitPool.find(msg.hash)->second;
        for(auto msg = msgSet.begin(); msg != msgSet.end() && i < MIN_APPROVALS; msg++) {
            messages[i++] = *msg;
        }

        // broadcastSignedBlock(hosts, block, messages);
        vector<Block> blocks = {block, block, block};
        vector<array<SignedMessage, MIN_APPROVALS>> messageSets = {messages, messages, messages};

        proposeFinal(blocks, messageSets);
    }

    SignedMessage roundChangeMsg = createPBFTMessage(msg.hash, user.getPublicKey(), user.getPrivateKey(), ROUND_CHANGE);
    insertIntoPool(roundChangePool, roundChangeMsg);
    broadcastMessage(hosts, roundChangeMsg);
}

void PBFTManager::roundChange(SignedMessage msg) {
    if(poolHasMessage(roundChangePool, msg))
        return;

    Logger::logStatus("Round change called!");

    bool isValid = checkSignature((const char*)msg.hash.data(), msg.hash.size(), msg.signature, msg.publicKey);
    if(!isValid)
        return;

    insertIntoPool(roundChangePool, msg);
    broadcastMessage(hosts, msg);

    if(messagePoolSize(roundChangePool, msg) >= MIN_APPROVALS) {
        Logger::logStatus("End of the line! clear out the pools");
        blockPool.clear();
    }
}

// PBFT functionality for the final commit

SHA256Hash buildBlockDigest(vector<Block> blocks) {
    vector<SHA256Hash> hashes;
    for(Block block : blocks) 
        hashes.push_back(block.getHash());
    sort(hashes.begin(), hashes.end());
    string digestInput = "";
    for(SHA256Hash hash : hashes)
        digestInput.append(SHA256toString(hash));
    SHA256Hash digest = SHA256(digestInput);
    return digest;
}

void PBFTManager::proposeFinal(vector<Block> blocks, vector<array<SignedMessage, MIN_APPROVALS>> signatures) {
    if(this->digest != NULL_SHA256_HASH)
        return;

    Logger::logStatus("Propose final called!");
    
    if(blocks.size() != signatures.size()) {
        Logger::logStatus("Bad number of blocks, giving up.");
        return;
    }

    for(int i = 0; i < blocks.size(); i++) {
        Block block = blocks.at(i);
        auto msgSet = signatures.at(i);
        for(int j = 0; j < MIN_APPROVALS; j++) {
            SignedMessage msg = msgSet[j];
            bool isValid = checkSignature((const char*)block.getHash().data(), msg.hash.size(), msg.signature, msg.publicKey);
            if(!isValid) {
                Logger::logStatus("Signature not valid, giving up.");
                return;
            }
        }
    }

    // create digest and random number
    this->digest = buildBlockDigest(blocks);
    this->random = randomString(32);
    SHA256Hash randHash = SHA256(this->random);
    broadcastSignedBlocks(hosts, blocks, signatures);
    
    SignedMessage msg = createPBFTMessage(this->digest, user.getPublicKey(), user.getPrivateKey(), PREPARING_F);
    insertIntoPool(preparePoolFinal, msg);
    broadcastMessage(hosts, msg);
}

void PBFTManager::prepareFinal(SignedMessage msg) {
    if(poolHasMessage(preparePoolFinal, msg))
        return;

    Logger::logStatus("Prepare final called!");

    bool isValid = checkSignature((const char*)msg.hash.data(), msg.hash.size(), msg.signature, msg.publicKey);
    if(!isValid)
        return;

    insertIntoPool(preparePoolFinal, msg);
    broadcastMessage(hosts, msg);

    if(messagePoolSize(preparePoolFinal, msg) < MIN_APPROVALS)
        return;

    SignedMessage commitMsg = createPBFTMessage(msg.hash, user.getPublicKey(), user.getPrivateKey(), COMMITTING_F);
    insertIntoPool(commitPoolFinal, commitMsg);
    broadcastMessage(hosts, commitMsg);
}

void PBFTManager::commitFinal(SignedMessage msg) {
    if(poolHasMessage(commitPoolFinal, msg))
        return;

    Logger::logStatus("Commit final called!");

    bool isValid = checkSignature((const char*)msg.hash.data(), msg.hash.size(), msg.signature, msg.publicKey);
    if(!isValid)
        return;

    insertIntoPool(commitPoolFinal, msg);
    broadcastMessage(hosts, msg);

    if(messagePoolSize(commitPoolFinal, msg) >= MIN_APPROVALS) {
        Logger::logStatus("WOW WE DID IT COMMIT THE BLOCK MY DUDE ;OO!");
    }

    SignedMessage roundChangeMsg = createPBFTMessage(msg.hash, user.getPublicKey(), user.getPrivateKey(), ROUND_CHANGE_F);
    insertIntoPool(roundChangePoolFinal, roundChangeMsg);
    broadcastMessage(hosts, roundChangeMsg);
}

void PBFTManager::roundChangeFinal(SignedMessage msg) {
    if(poolHasMessage(roundChangePoolFinal, msg))
        return;

    Logger::logStatus("Round change final called!");

    bool isValid = checkSignature((const char*)msg.hash.data(), msg.hash.size(), msg.signature, msg.publicKey);
    if(!isValid)
        return;

    insertIntoPool(roundChangePoolFinal, msg);
    broadcastMessage(hosts, msg);

    if(messagePoolSize(roundChangePoolFinal, msg) >= MIN_APPROVALS) {
        Logger::logStatus("End of the line! clear out the pools");
        this->digest = NULL_SHA256_HASH;
    }
}
