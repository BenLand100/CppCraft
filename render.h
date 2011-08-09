#ifndef _render
#define _render

#include "client.h"

class Back2Front {
    public:
    float px,py,pz;
    inline Back2Front(float _px,float _py,float _pz) : px(_px),py(_py),pz(_pz) { }
    inline bool operator()(const Pos3D &a, const Pos3D &b) {
        return ((a.cx-px)*(a.cx-px)+(a.cy-py)*(a.cy-py)+(a.cz-pz)*(a.cz-pz)) >= ((b.cx-px)*(b.cx-px)+(b.cy-py)*(b.cy-py)+(b.cz-pz)*(b.cz-pz));
    }
};

bool initRender();
void renderWorld(Client *client);
void processEvents(Client *client);
void quitRender();
void disposeChunk(Chunk *chunk);

#endif
