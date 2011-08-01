#ifndef _world
#define _world

//Coordinates of the chunk that contains the position
inline void chunkPos(int x, int y, int z, int &cx, int &cy, int &cz) {
    cx = x >> 4;
    cy = y >> 7;
    cz = z >> 4;
}

//Coordinates of the chunk that contains the position
inline void chunkPos(double x, double y, double z, int &cx, int &cy, int &cz) {
    cx = ((int)x) >> 4;
    cy = ((int)y) >> 7;
    cz = ((int)z) >> 4;
}

//Local (inside chunk) coordinates of the position
inline void localPos(int x, int y, int z, int &lx, int &ly, int &lz) {
    lx = x & 15;
    ly = y & 127;
    lz = z & 15;
}

//Local (inside chunk) coordinates of the position
inline void localPos(double x, double y, double z, int &lx, int &ly, int &lz) {
    lx = ((int)x) & 15;
    ly = ((int)y) & 127;
    lz = ((int)z) & 15;
}

//Block position of the double position (cast to int)
inline void blockPos(double x, double y, double z, int &bx, int &by, int &bz) {
    bx = (int)x;
    by = (int)y;
    bz = (int)z;
}

class Block {
    public:
        Block();
        ~Block();
        
        int type;
};

class Chunk {
    public:
        Chunk();
        ~Chunk();
        
        bool update(int lx, int ly, int lz, int sx, int sy, int sz, int size, char *cdata);
        
        inline Block* getBlock(int lx, int ly, int lz) {
            return &blocks[lx][lz][ly];
        }
        
    private:
        Block blocks[16][16][128]; //[X][Z][Y] for speed...

};

class World {
    public:
        World();
        ~World();
        
        Block* getBlock(int x, int y, int z); //get block if it exists (if many are needed, use getChunk, it's faster)
        Chunk* getChunk(int x, int y, int z); //get chunk containing block if it exists
        
        void resetChunks();
        
        bool initChunk(int cx, int cy, int cz); //does nothing since not always sent anyway
        bool updateChunk(int x, int y, int z, int sx, int sy, int sz, int size, char *cdata); //position in world coords. unknown if it can cross chunk boundaries (this will fail if it does)
        bool deleteChunk(int cx, int cy, int cz); 

};

#endif
