#include <unordered_set>
#include "../core/user.hpp"
using namespace std;

struct SignedMessage {
    SHA256Hash hash;
    PublicKey publicKey;
    // technically signs a block hash but this is okay
    TransactionSignature signature;
};

struct BlockHash {
public:
	size_t operator()(const Block b) const 
    {
		return std::hash<uint32_t>()(b.id);
	}
};

// struct MessageHash {
// public:
// 	size_t operator()(const SignedMessage msg) const 
//     {
// 		return std::hash<uint32_t>()(stoi(signatureToString(msg.signature)));
// 	}
// 	size_t operator==(const SignedMessage a) const 
//     {
// 		return signatureToString(a.signature);
// 	}
// };

enum PBFTState {
    IDLE,
    PROPOSING,
    PREPREPARING,
    PREPARING,
    COMMITTING,
};

class PBFTManager {
    public:
        PBFTManager(HostManager& h, BlockChain& b, MemPool& m);
        void proposeBlock();
        void prePrepare(Block& b);
        void prepare(SignedMessage msg);
        void commit(SignedMessage msg);
        void roundChange(SignedMessage msg);
    private:
        bool isProposer();
        Block createBlock();
        BlockChain& blockchain;
        HostManager& hosts;
        MemPool& mempool;
        User user;
        unordered_set<Block, BlockHash> blockPool;
        // unordered_set<SignedMessage, MessageHash> preparePool;
        // unordered_set<SignedMessage, MessageHash> commitPool;
        PBFTState state;
};