#include "../core/logger.hpp"
#include "../core/host_manager.hpp"
#include "../core/block.hpp"
#include "blockchain.hpp"
#include "pbft.hpp"

PBFTManager::PBFTManager(HostManager& h, BlockChain& b) : hosts(h), blockchain(b) {
    Logger::logStatus("PBFTManager initialized!");

    // blockPool;
    //create block pool, prepare message pool, & commit pool
}

void PBFTManager::proposeBlock() {
    Logger::logStatus("Block proposal called!");
    // create a block from transaction queue
    // broadcast preprepare message with the block
};

void PBFTManager::prePrepare() {
    Logger::logStatus("PrePrepare called!");
    // check validity of the preprepare block
    // if valid add this block to the blockpool
    // broadcast preprepare message with the block
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