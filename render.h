#ifndef _render
#define _render

#include "client.h"

class Back2Front {
    public:
    int px,py,pz;
    inline Back2Front(int _px,int _py,int _pz) : px(_px),py(_py),pz(_pz) { }
    inline bool operator()(const ChunkPos &a, const ChunkPos &b) {
        return ((a.cx-px)*(a.cx-px)+(a.cy-py)*(a.cy-py)+(a.cz-pz)*(a.cz-pz)) >= ((b.cx-px)*(b.cx-px)+(b.cy-py)*(b.cy-py)+(b.cz-pz)*(b.cz-pz));
    }
};

bool initRender();
void renderWorld(Client *client);
void processEvents(Client *client);
void quitRender();
void disposeChunk(Chunk *chunk);

#endif
