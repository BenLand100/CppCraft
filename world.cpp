#include "world.h"
#include <iostream>

Block::Block() : type(0) {

}

Block::~Block() {

}

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

bool Chunk::update(int lx, int ly, int lz, int sx, int sy, int sz, int size, char *cdata) {
    //std::cout << "Parse chunk data...\n";
    return false;
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
    int lx,ly,lz;
    chunkPos(x,y,z,lx,ly,lz);
    Chunk *c = getChunk(x,y,z);
    if (!c) {
        lock();
        int cx,cy,cz;
        chunkPos(x,y,z,cx,cy,cz);
        std::cout << "Creating Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ") " << chunks.size() << '\n';
        c = new Chunk();
        chunks[ChunkPos(cx,cy,cz)] = c;
        unlock();
    }
    return c->update(lx,ly,lz,sx,sy,sz,size,cdata);
}

bool World::deleteChunk(int cx, int cy, int cz) {
    lock();
    std::map<ChunkPos,Chunk*>::iterator ci = chunks.find(ChunkPos(cx,cy,cz));
    if (ci == chunks.end()) return false;
    //std::cout << "Deleting Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ")\n";
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

