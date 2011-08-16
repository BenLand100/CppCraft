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

#ifndef _render
#define _render

#include "client.h"
#include <map>

class Back2Front {
    public:
    float px,py,pz;
    inline Back2Front(float _px,float _py,float _pz) : px(_px),py(_py),pz(_pz) { }
    inline bool operator()(const Pos3D &a, const Pos3D &b) {
        return ((a.cx-px)*(a.cx-px)+(a.cy-py)*(a.cy-py)+(a.cz-pz)*(a.cz-pz)) >= ((b.cx-px)*(b.cx-px)+(b.cy-py)*(b.cy-py)+(b.cz-pz)*(b.cz-pz));
    }
};

typedef struct {
    bool haslist;
    int list;
    std::map<Pos3D,Block*,Back2Front> *translucent;
} RenderData;

bool initRender();
void renderWorld(Client *client);
void processEvents(Client *client);
void quitRender();
void disposeChunk(Chunk *chunk);

#endif
