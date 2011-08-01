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
    std::cout << "Parse chunk data...\n";
    return false;
}

World::World() {

}

World::~World() {

}

Block* World::getBlock(int x, int y, int z) {
    int lx,ly,lz;
    Chunk *c = getChunk(x,y,z);
    localPos(x,y,z,lx,ly,lz);
    return c ? c->getBlock(lx,ly,lz) : NULL;
}

Chunk* World::getChunk(int x, int y, int z) {
    return NULL;
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
        int cx,cy,cz;
        chunkPos(x,y,z,cx,cy,cz);
        std::cout << "Creating Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ")\n";
        c = new Chunk();
    }
    return c->update(lx,ly,lz,sx,sy,sz,size,cdata);
}

bool World::deleteChunk(int cx, int cy, int cz) {
    std::cout << "Removing Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ")\n";
    return false;
}

void World::resetChunks() {

}

