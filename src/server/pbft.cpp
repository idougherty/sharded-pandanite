#include "../core/logger.hpp"
#include "../core/host_manager.hpp"
#include "../core/block.hpp"
#include "blockchain.hpp"
#include "mempool.hpp"
#include "pbft.hpp"

void broadcastMessage(HostManager& hm, SignedMessage msg) {
    std::vector<string> hosts = hm.getHosts();
    for(string host : hosts) {
        Logger::logStatus("Messaging " + host);
    }
}

void broadcastBlock(HostManager& hm, Block block) {
    Logger::logStatus("Broadcasting! 1");
    std::vector<string> hosts = hm.getHosts();
    Logger::logStatus("Broadcasting! 2");
    for(string host : hosts) {
        Logger::logStatus("Messaging " + host);
    }

    // json stats = hm.getHeaderChainStats();
    // for (auto& peer : stats.items()) {
    //     Logger::logStatus(peer.key());
    // }

}

PBFTManager::PBFTManager(HostManager& h, BlockChain& b, MemPool& m) : hosts(h), blockchain(b), mempool(m) {
    Logger::logStatus("PBFTManager initialized!");
    state = IDLE;
}

void PBFTManager::proposeBlock() {
    if(state == PROPOSING) return;
    state = PROPOSING;

    Logger::logStatus("Block proposal called!");

    Block block = createBlock();

    Logger::logStatus("Created new block!");

    broadcastBlock(hosts, block);
    Logger::logStatus("Broadcasted block!");
};

void PBFTManager::prePrepare() {
    Logger::logStatus("PrePrepare called!");
    // check validity of the preprepare block
    // if valid add this block to the blockpool

    // broadcast preprepare message with the block
    // SignedMessage msg = createPBFTMessage();
    // broadcastMessage(hosts, msg);
    
    // sign the block and add it to the prepare pool
    // broadcast prepare message with signed block
};

void PBFTManager::prepare() {
    Logger::logStatus("Prepare called!");
    // check validity of the prepare message
    // if valid add message to the prepare pool
    // broadcast the prepare message
    // if the prepare pool is bigger than min approvals 
    // then add data to commit pool and broadcast commit
}

void PBFTManager::commit() {
    Logger::logStatus("Commit called!");
    // check validity of the commit message
    // if valid add message to the commitpool
    // broadcast the commit message
    // if commit pool is bigger than min approvals
    // then commit the block to the chain and broadcast round change
}

// do we need this step?
void PBFTManager::roundChange() {
    Logger::logStatus("Round change called!");
    // check validity of the round change message
    // add to message pool and broadcast
    // if message pool is bigger min approvals
    // then clear the transaction queue
}

Block PBFTManager::createBlock() {
    Block newBlock;

    int chainLength = blockchain.getBlockCount();
    int challengeSize = blockchain.getDifficulty();
    SHA256Hash lastHash = blockchain.getLastHash();

    uint64_t lastTimestamp = std::time(0);
    lastTimestamp = blockchain.getBlockHeader(chainLength).timestamp;

    // check that our mined blocks timestamp is *at least* as old as the tip of the chain.
    // if it's not then your system clock is wonky, so we just make up a date:
    if (newBlock.getTimestamp() < lastTimestamp) {
        newBlock.setTimestamp(lastTimestamp + 1);
    }

    std::vector<Transaction> txs = mempool.getTransactions();
    for(auto tx : txs) {
        newBlock.addTransaction(tx);
    }

    newBlock.setId(chainLength + 1);

    if (newBlock.getTimestamp() < lastTimestamp) {
        newBlock.setTimestamp(lastTimestamp);
    }

    MerkleTree m;
    m.setItems(newBlock.getTransactions());
    newBlock.setMerkleRoot(m.getRootHash());
    newBlock.setDifficulty(challengeSize);
    newBlock.setLastBlockHash(lastHash);

    SHA256Hash solution = mineHash(newBlock.getHash(), challengeSize, newBlock.getId() > PUFFERFISH_START_BLOCK);
    newBlock.setNonce(solution);

    return newBlock;
}

SignedMessage createPBFTMessage(SHA256Hash hash, PublicKey pub, PrivateKey priv) {
    SignedMessage msg;
    msg.hash = hash;
    msg.publicKey = pub;
    msg.signature = signWithPrivateKey(SHA256toString(hash), pub, priv);

    return msg;
}