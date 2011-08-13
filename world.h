#ifndef _world
#define _world

#include "SDL_mutex.h"
#include <map>
#include "items.h"

class Client; //foward declaration

inline void worldPos(int cx, int cy, int cz, int &x, int &y, int &z) {
    x = cx << 4;
    y = cy << 7;
    z = cz << 4;
}

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

class Pos3D {
    public:
        int cx,cy,cz;
        inline Pos3D(int _cx, int _cy, int _cz) : cx(_cx), cy(_cy), cz(_cz) { }
        inline bool operator<(const Pos3D &pos) const {
            //damn strict weak ordering
            if (cx != pos.cx) return (cx < pos.cx);
            if (cz != pos.cz) return (cz < pos.cz);
            return (cy < pos.cy);
        }
        inline bool operator==(const Pos3D &pos) const {
            return (pos.cx == cx) && (pos.cz == cz) && (pos.cy == cy);
        }
};

#define O_AIR           0
#define O_TRANSLUCENT   1
#define O_OPAQUE        2

#define S_BLOCK         3
#define S_DECORATION    4
#define S_SPECIAL       5

class Block {
    public:
        inline Block() : type(0), meta(0), light(0), sky(15) { }
        inline ~Block() { }
        
        //see defines above
        int opacity();
        int style();
        
        inline const char* name() { return item_name(type); }
        int maxstack() { return item_maxstack(type); }
        bool placeable() { return item_placeable(type); }
        bool usable() { return item_usable(type); }
        bool passable() { return item_passable(type); }
        bool diggable() { return item_diggable(type); }
        bool activatable() { return item_activatable(type); }
        bool safe() { return item_safe(type); }
        float hardness() { return item_hardness(type); }
        int material() { return item_material(type); }
        
        int type;
        char meta,light,sky;
};

//forward decleration
class Back2Front;

class Chunk {
    public:
        Chunk();
        ~Chunk();
        
        bool update(int lx, int ly, int lz, int sx, int sy, int sz, int size, char *cdata);
        bool update(int size, short *locs, char *types, char *metas);
        void markDirty();
        void markBoundaryDirty();
        
        inline Block* getBlock(int lx, int ly, int lz) {
            return &blocks[(lx*16+lz)*128+ly];
        }
        
    private:
        Block blocks[16*16*128]; //packed [X][Z][Y] for speed...
        
        bool dirty,boundarydirty;
        void *renderdata;
        
        void (*destroy)(Chunk *us);

    friend void drawStaticChunk(Chunk *chunk, int cx, int cy, int cz, Chunk *ctop, Chunk *cbottom, Chunk *cright, Chunk *cleft, Chunk *cfront, Chunk *cback, std::map<Pos3D,Block*,Back2Front> &translucent);
    friend void drawTranslucentChunk(Chunk *chunk, int cx, int cy, int cz, Chunk *ctop, Chunk *cbottom, Chunk *cright, Chunk *cleft, Chunk *cfront, Chunk *cback, std::map<Pos3D,Block*,Back2Front> &translucent);
    friend void renderWorld(Client *client);
    friend void disposeChunk(Chunk *chunk);
};

#define F_NONE      -1
#define F_MINUS_Y   0
#define F_PLUS_Y    1
#define F_MINUS_Z   2   
#define F_PLUS_Z    3
#define F_MINUS_X   4
#define F_PLUS_X    5


class World {
    public:
        World();
        ~World();
        
        Block* getBlock(int x, int y, int z); //get block if it exists (if many are needed, use getChunk, it's faster)
        Chunk* getChunk(int x, int y, int z); //get chunk containing block if it exists
        Chunk* getChunkIdx(int cx, int cy, int cz); //get chunk by the chunk's coordinates
        
        void clearChunks();
        
        bool initChunk(int cx, int cy, int cz); //does nothing since not always sent anyway
        bool updateChunk(int x, int y, int z, int sx, int sy, int sz, int size, char *cdata); //position in world coords. unknown if it can cross chunk boundaries (this will fail if it does)
        bool updateChunk(int cx, int cy, int cz, int size, short *locs, char *types, char *metas); //position in chunk coords
        bool deleteChunk(int cx, int cy, int cz);
        
        void updateLighting(int x, int y, int z);
        
        void lockChunks();
        void unlockChunks();
        
        //checks if a non-passable block is in the range 
        bool containsSolid(int sx,int sy,int sz,int ex,int ey,int ez);
        //fills fx,fy,fz with a normalized vector representing where the camera is looking
        void facingNormal(double pitch, double yaw, double &fx, double &fy, double &fz);
        //finds the first non-air block intersected by the line of sight and the face that is incident (returns NULL if none found)
        Block* projectToBlock(double px, double py, double pz, double pitch, double yaw, int &x, int &y, int &z, int &face);
        
        
    private:
        std::map<Pos3D,Chunk*> chunks;
        SDL_mutex *chunklock;
        
    friend void renderWorld(Client *client);
};

#endif
