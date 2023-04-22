
struct prepareMessage {
    SHA256Hash blockhash;
    PublicKey publicKey;
    // technically signs a block hash but this is okay
    TransactionSignature signature;
};

class PBFTManager {
    public:
        PBFTManager(HostManager& h, BlockChain& b);
        void proposeBlock();
        void prePrepare();
        void prepare();
        void commit();
        void roundChange();
    private:
        BlockChain& blockchain;
        HostManager& hosts;
        std::set<Block> blockPool;
};