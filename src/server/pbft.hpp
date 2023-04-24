#include <unordered_set>
#include "../core/user.hpp"
#include "../core/block.hpp"
using namespace std;

struct BlockHash {
public:
	size_t operator()(const Block b) const 
    {
		return std::hash<string>()(SHA256toString(b.getHash()));
	}
};

class PBFTManager {
    public:
        PBFTManager(HostManager& h, BlockChain& b, MemPool& m);
        // void proposeBlock();
        void prePrepare(Block& b);
        void prepare(SignedMessage msg);
        void commit(SignedMessage msg);
        void roundChange(SignedMessage msg);
    private:
        // bool isProposer();
        // Block createBlock();
        BlockChain& blockchain;
        HostManager& hosts;
        MemPool& mempool;
        User user;
        unordered_set<Block, BlockHash> blockPool;
        unordered_set<SignedMessage, MessageHash> preparePool;
        unordered_set<SignedMessage, MessageHash> commitPool;
        PBFTState state;
};