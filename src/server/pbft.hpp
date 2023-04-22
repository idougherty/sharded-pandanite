
struct SignedMessage {
    SHA256Hash hash;
    PublicKey publicKey;
    // technically signs a block hash but this is okay
    TransactionSignature signature;
};

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
        void prePrepare();
        void prepare();
        void commit();
        void roundChange();
    private:
        Block createBlock();
        BlockChain& blockchain;
        HostManager& hosts;
        MemPool& mempool;
        std::set<Block> blockPool;
        std::set<SignedMessage> preparePool;
        std::set<SignedMessage> commitPool;
        PBFTState state;
};