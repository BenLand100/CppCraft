#include "world.h"
#include <iostream>
#include "zlib.h"
#include "render.h"


Chunk::Chunk(SDL_mutex *lock) : worldlock(lock), dirty(true), haslist(false) {
}

Chunk::~Chunk() {
    SDL_mutexP(worldlock);
    disposeChunk(this);
    SDL_mutexV(worldlock);
}

void Chunk::markDirty() {
    dirty = true;
}

bool Chunk::update(int lx, int ly, int lz, int sx, int sy, int sz, int size, char *cdata) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit(&strm);
    if (ret != Z_OK) return false;
    
    strm.avail_in = size;
    strm.next_in = (unsigned char*) cdata;
    
    sx++; sy++; sz++; //why? minecraft. 
    int len = (int) (sz*sy*sz*2.5);
    unsigned char *data = new unsigned char[len];
    
    strm.avail_out = len;
    strm.next_out = data;
    
    do {
        ret = inflate(&strm, Z_NO_FLUSH);
    } while (strm.avail_out != 0 && ret == Z_OK);
    inflateEnd(&strm);
    if (ret != Z_OK && ret != Z_STREAM_END) return false;
    
    //yea, they sub-byte-packed shit, cause THAT compresses well -.-
    unsigned char *types = data;
    unsigned char *metas = &data[sx*sy*sz];
    unsigned char *lights = &data[sx*sy*sz*3/2];
    unsigned char *skys = &data[sx*sy*sz*4/2];
    int i = 0;
    for (int x = lx; x < lx+sx; x++) {
        Block *slice = &blocks[x*16*128];
        for (int z = lz; z < lz+sz; z++) {
            Block *col = &slice[z*128];
            for (int y = ly; y < ly+sy; y++) {
                col[y].type = *(types++);
                if (i&1) {
                    col[y].meta = (*(metas++) >> 4) & 0xF;
                    col[y].light = (*(lights++) >> 4) & 0xF;
                    col[y].sky = (*(skys++) >> 4) & 0xF;
                } else {
                    col[y].meta = (*(metas)) & 0xF;
                    col[y].light = (*(lights)) & 0xF;
                    col[y].sky = (*(skys)) & 0xF;
                }
                i++;
            }
        }
    }
    dirty = true;
    
    delete data;
    return true;
}

bool Chunk::update(int size, short *locs, char *types, char *metas) {
    for (int i = 0; i < size; i++) {
        int x = (*locs >> 12) & 0xF;
        int z = (*locs >> 8) & 0xF;
        int y = *(locs++) & 0xFF;
        blocks[(x*16+z)*128+y].type = *(types++);
        blocks[(x*16+z)*128+y].meta = *(metas++);
    }
    dirty = true;
    return true;
}

World::World() {
    chunklock = SDL_CreateMutex();
}

World::~World() {
    SDL_DestroyMutex(chunklock);
}

void World::lock() {
    SDL_mutexP(chunklock);
}


void World::unlock() {
    SDL_mutexV(chunklock);
}

Block* World::getBlock(int x, int y, int z) {
    int lx,ly,lz;
    Chunk *c = getChunk(x,y,z);
    localPos(x,y,z,lx,ly,lz);
    return c ? c->getBlock(lx,ly,lz) : NULL;
}

Chunk* World::getChunk(int x, int y, int z) {
    int cx,cy,cz;
    chunkPos(x,y,z,cx,cy,cz);
    lock();
    std::map<ChunkPos,Chunk*>::iterator ci = chunks.find(ChunkPos(cx,cy,cz));
    Chunk *c = NULL;
    if (ci != chunks.end()) {
        c = ci->second;
    }
    unlock();
    return c;
}

bool World::initChunk(int cx, int cy, int cz) {
    //does nothing because this is aparantly not always sent by the server
    return true;
}

bool World::updateChunk(int x, int y, int z, int sx, int sy, int sz, int size, char *cdata) {
    Chunk *c = getChunk(x,y,z);
    if (!c) {
        lock();
        int cx,cy,cz;
        chunkPos(x,y,z,cx,cy,cz);
        std::cout << "Creating Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ") " << chunks.size() << '\n';
        c = new Chunk(chunklock);
        chunks[ChunkPos(cx,cy,cz)] = c;
        unlock();
    }
    int lx,ly,lz;
    localPos(x,y,z,lx,ly,lz);
    return c->update(lx,ly,lz,sx,sy,sz,size,cdata);
}

bool World::updateChunk(int cx, int cy, int cz, int size, short *locs, char *types, char *metas) {
    int x,y,z; worldPos(cx,cy,cz,x,y,z);
    Chunk *c = getChunk(x,y,z);
    if (!c) return false; //this cannot create a new chunk
    return c->update(size,locs,types,metas);
}

bool World::deleteChunk(int cx, int cy, int cz) {
    lock();
    std::map<ChunkPos,Chunk*>::iterator ci = chunks.find(ChunkPos(cx,cy,cz));
    if (ci == chunks.end()) return false;
    std::cout << "Deleting Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ")\n";
    delete ci->second;
    chunks.erase(ci);
    unlock();
    return true;
}

void World::clearChunks() {
    lock();
    std::map<ChunkPos,Chunk*>::iterator ci = chunks.begin();
    std::map<ChunkPos,Chunk*>::iterator end = chunks.end();
    int i = 0;
    for ( ; ci != end; ci++, i++) {
        delete ci->second;
    }
    chunks.clear();
    unlock();
}

