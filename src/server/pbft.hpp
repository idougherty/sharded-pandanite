// #include <unordered_map>
#include <unordered_set>
#include "../core/user.hpp"
#include "../core/block.hpp"
#include "../core/crypto.hpp"
using namespace std;

struct MapHash {
public:
	size_t operator()(const SHA256Hash h) const 
    {
		return std::hash<string>()(SHA256toString(h));
	}
};

typedef unordered_map<SHA256Hash, unordered_set<SignedMessage, MessageHash>, MapHash> MessagePool;

class PBFTManager {
    public:
        PBFTManager(HostManager& h, BlockChain& b, MemPool& m);
        void prePrepare(Block& b);
        void prepare(SignedMessage msg);
        void commit(SignedMessage msg);
        void roundChange(SignedMessage msg);

        void proposeFinal(vector<Block> blocks, vector<array<SignedMessage, MIN_APPROVALS>> signatures);
        void prepareFinal(SignedMessage msg);
        void commitFinal(SignedMessage msg);
        void roundChangeFinal(SignedMessage msg);

    private:
        bool poolHasMessage(MessagePool& pool, SignedMessage msg);
        void insertIntoPool(MessagePool& pool, SignedMessage msg);
        size_t messagePoolSize(MessagePool& pool, SignedMessage msg);
        BlockChain& blockchain;
        HostManager& hosts;
        MemPool& mempool;
        User user;

        unordered_map<SHA256Hash, Block, MapHash> blockPool;
        MessagePool preparePool;
        MessagePool commitPool;
        MessagePool roundChangePool;

        string random;
        SHA256Hash digest;
        unordered_set<SHA256Hash, MapHash> randPool;
        MessagePool preparePoolFinal;
        MessagePool commitPoolFinal;
        MessagePool roundChangePoolFinal;
};