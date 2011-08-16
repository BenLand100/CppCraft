/**
 *  Copyright 2011 by Benjamin J. Land (a.k.a. BenLand100)
 *
 *  This file is part of the CppCraft.
 *
 *  CppCraft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CppCraft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CppCraft. If not, see <http://www.gnu.org/licenses/>.
 */

#include "world.h"
#include <iostream>
#include <cmath>
#include "zlib.h"
#include "render.h"

//decorations and specials must be translucent as well
int Block::opacity() {
    switch (type) {
        case 0: return O_AIR;
        case 31: case 32: case 37: case 38: 
        case 9: case 18: case 20: case 83: return O_TRANSLUCENT;
        default: return O_OPAQUE;
    }
}


int Block::style() {
    switch (type) {
        case 31: case 32: case 37: case 38: case 83: return S_DECORATION;
        default: return S_BLOCK;
    }
}

Chunk::Chunk() : dirty(true),boundarydirty(false), renderdata(NULL), destroy(NULL) {
}

Chunk::~Chunk() {
    if (destroy) destroy(this);
}

void Chunk::markDirty() {
    dirty = true;
}

void Chunk::markBoundaryDirty() {
    boundarydirty = true;
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
    
    sx++; sy++; sz++; 
    int nibblelen = sx*sy*sz/2;
    int len = nibblelen*5;
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
    unsigned char *metas = &data[nibblelen*2];
    unsigned char *lights = &data[nibblelen*3];
    unsigned char *skys = &data[nibblelen*4];
    int i = 0;
    for (int x = lx; x < sx+lx; x++) {
        Block *slice = &blocks[x*16*128];
        for (int z = lz; z < sz+lz; z++) {
            Block *col = &slice[z*128];
            for (int y = ly; y < sy+ly; y++) {
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

void World::lockChunks() {
    SDL_mutexP(chunklock);
}


void World::unlockChunks() {
    SDL_mutexV(chunklock);
}

void World::facingNormal(double pitch, double yaw, double &fx, double &fy, double &fz) {
    fx = cos(-pitch/180.0*3.14159)*cos(yaw/180.0*3.14159);
    fz = cos(-pitch/180.0*3.14159)*sin(yaw/180.0*3.14159);
    fy = sin(-pitch/180.0*3.14159);
}

Block* World::projectToBlock(double px, double py, double pz, double pitch, double yaw, int &x, int &y, int &z, int &face) {
    double fx,fy,fz; facingNormal(pitch,yaw,fx,fy,fz);
    int sx = (int)px; int ix = fx < 0 ? -1 : 1, ex = sx + 4*ix;
    int sy = (int)py; int iy = fy < 0 ? -1 : 1, ey = sy + 4*iy;
    int sz = (int)pz; int iz = fz < 0 ? -1 : 1, ez = sz + 4*iz;
    double tmin = 6.0; //max distance to consider
    Block *res = NULL;
    for (int cx = sx; cx != ex; cx += ix) {
        double t = (cx-px)/fx;
        if (t > 0 && t <= tmin) {
            int tx = fx > 0 ? cx : cx-1;
            int ty = (int)floor(py+fy*t);
            int tz = (int)floor(pz+fz*t);
            Block *b = getBlock(tx,ty,tz);
            if (b && b->type) {
                tmin = t;
                x = tx;
                y = ty;
                z = tz;
                face = fx > 0 ? F_MINUS_X : F_PLUS_X;
                res = b;
            }
        }
    }
    for (int cy = sy; cy != ey; cy += iy) {
        double t = (cy-py)/fy;
        if (t > 0 && t <= tmin) {
            int tx = (int)floor(px+fx*t);
            int ty = fy > 0 ? cy : cy-1;
            int tz = (int)floor(pz+fz*t);
            Block *b = getBlock(tx,ty,tz);
            if (b && b->type) {
                tmin = t;
                x = tx;
                y = ty;
                z = tz;
                face = fy > 0 ? F_MINUS_Y : F_PLUS_Y;
                res = b;
            }
        }
    }
    for (int cz = sz; cz != ez; cz += iz) {
        double t = (cz-pz)/fz;
        if (t > 0 && t <= tmin) {
            int tx = (int)floor(px+fx*t);
            int ty = (int)floor(py+fy*t);
            int tz = fz > 0 ? cz : cz-1;
            Block *b = getBlock(tx,ty,tz);
            if (b && b->type) {
                tmin = t;
                x = tx;
                y = ty;
                z = tz;
                face = fz > 0 ? F_MINUS_Z : F_PLUS_Z;
                res = b;
            }
        }
    }
    return res;
}

bool World::containsSolid(int sx,int sy,int sz,int ex,int ey,int ez) {
    int cx, cy, cz, tcx, tcy, tcz,lx,ly,lz;
    chunkPos(sx,sy,sz,cx,cy,cz);
    Chunk *c = getChunk(sx,sy,sz);
    for (int x = sx; x <= ex; x++) {
        for (int z = sz; z <= ez; z++) {
            for (int y = sy; y <= ey; y++) {
                chunkPos(x,y,z,tcx,tcy,tcz);
                if (tcx != cx || tcy != cy || tcz != cz) {
                    cx = tcx; cy = tcy; cz = tcz;
                    c = getChunk(x,y,z);
                }
                if (!c) continue;
                localPos(x,y,z,lx,ly,lz);
                if (!c->getBlock(lx,ly,lz)->passable()) return true;
            }
        }
    }
    return false;
}

void World::updateLighting(int x, int y, int z) {
    //TODO make the lighting update... will be tedious and the fomula is sort of unknown
    Chunk *c = getChunk(x,y,z); if (c) c->markDirty();
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
    return getChunkIdx(cx,cy,cz);
}

Chunk* World::getChunkIdx(int cx, int cy, int cz) {
    std::map<Pos3D,Chunk*>::iterator ci = chunks.find(Pos3D(cx,cy,cz));
    Chunk *c = NULL;
    if (ci != chunks.end()) {
        c = ci->second;
    }
    return c;
}

bool World::initChunk(int cx, int cy, int cz) {
    //does nothing because this is aparantly not always sent by the server
    return true;
}

bool World::updateChunk(int x, int y, int z, int sx, int sy, int sz, int size, char *cdata) {
    Chunk *c = getChunk(x,y,z);
    if (!c) {
        lockChunks();
        int cx,cy,cz;
        chunkPos(x,y,z,cx,cy,cz);
        //std::cout << "Creating Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ") " << chunks.size() << '\n';
        c = new Chunk();
        chunks[Pos3D(cx,cy,cz)] = c;
        unlockChunks();
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
    lockChunks();
    std::map<Pos3D,Chunk*>::iterator ci = chunks.find(Pos3D(cx,cy,cz));
    if (ci == chunks.end()) {
        unlockChunks();
        return false;
    }
    //std::cout << "Deleting Chunk (" << std::dec << cx << ',' << cy << ',' << cz << ")\n";
    delete ci->second;
    chunks.erase(ci);
    unlockChunks();
    return true;
}

void World::clearChunks() {
    lockChunks();
    std::map<Pos3D,Chunk*>::iterator ci = chunks.begin();
    std::map<Pos3D,Chunk*>::iterator end = chunks.end();
    int i = 0;
    for ( ; ci != end; ci++, i++) {
        delete ci->second;
    }
    chunks.clear();
    unlockChunks();
}

