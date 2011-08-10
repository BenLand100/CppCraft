#include "render.h"
#include "world.h"
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_mouse.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "terrain.c"

#define w_width 800
#define w_height 600

float intensity[16];
int textureid;

GLvoid initGL(GLsizei width, GLsizei height) {
    glViewport(0,0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    /*glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_PERSPECTIVE_CORRECTION_HINT);*/
    
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_EMISSION);
    for (int i = 0; i < 16; i++) {
        intensity[15-i] = pow(0.8,i);
    }
    
    glBindTexture(GL_TEXTURE_2D, textureid);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,terrain.width,terrain.height,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8_REV,terrain.pixel_data);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,textureid);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(16.0f/terrain.width,16.0f/terrain.height,1.0f);
}

bool initRender() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to initialize SDL %s", SDL_GetError());
        return false;
    }
    if (SDL_SetVideoMode(w_width, w_height, 0, SDL_OPENGL) == NULL) {
        fprintf(stderr, "Unable to create openGL scene %s", SDL_GetError());
        return false;
    }
    //SDL_WM_GrabInput(SDL_GRAB_ON);
    //SDL_ShowCursor(SDL_DISABLE);
    initGL(w_width, w_height);
    return true;
}

//Face {0,1,2,3,4,5,6} == {-y,+y,-z,+z,-x,+x,decor}
//Result indicates static block if true (used in make display lists)
inline void setBlock(Block &block, Block &l, int face, int &tx, int &ty) {
    float r=1.0f,g=1.0f,b=1.0f;
    switch (block.type) {
        case 1:
            tx = 1; ty = 0; break;
        case 2:
            switch (face) {
                case 0: tx = 2; ty = 0; break;
                case 1: tx = 0; ty = 0; r = 0.0f; b = 0.0f; break;
                default: tx = 3; ty = 0; break;
            }
            break;
        case 3:
            tx = 2; ty = 0; break;
        case 4:
            tx = 0; ty = 1; break;
            
        case 9:
            tx = 13; ty = 12; break;
            
        case 12:
            tx = 2; ty = 1; break;
            
        case 17:
            switch (face) {
                case 0: case 1: tx = 5; ty = 1; break;
                default: 
                    switch (block.meta) {
                        case 0: tx = 4; ty = 1; break;
                        case 1: tx = 4; ty = 7; break;
                        case 2: tx = 5; ty = 7; break;
                        default: tx = 0; ty = 0; g = 0.0f; b = 0.0f; break;
                    } break;
            }
            break;
        case 18:
            switch (block.meta & 0x3) {
                case 0: tx = 4; ty = 3; r = 0.0f; b = 0.0f; break;
                case 1: tx = 4; ty = 8; r = 0.0f; g = 0.3f; b = 0.0f; break;
                case 2: tx = 4; ty = 8; r = 0.4f; b = 0.3f; break;
                default: tx = 0; ty = 0; g = 0.0f; b = 0.0f; break;
            }
            break;
            
        case 20:
            tx = 1; ty = 3; break;
            
        case 31: tx = 7; ty = 2; r = 0.2; b = 0.3; break;
        case 32: tx = 7; ty = 3;
        case 37: tx = 13; ty = 0; break;
        case 38: tx = 12; ty = 0; break;
            
        case 83: tx = 9; ty = 4; break;
        
        default:
            tx = 0; ty = 0; g = 0.0f; b = 0.0f;
            
    }
    float f = intensity[l.sky+l.light < 16 ? l.sky+l.light : 15];
    switch (face) {
        case 2: case 3:
            f *= 0.8; break;
        case 4: case 5:
            f *= 0.6; break;
        default:
            break;
    }
    glColor3f(r*f,g*f,b*f);
}

inline void drawTop(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    setBlock(b,l,1,tx,ty);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, z);
    glTexCoord2i(tx,ty);
    glVertex3i(x, 1+y, z);
    glTexCoord2i(tx,ty+1);
    glVertex3i(x, 1+y, 1+z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, 1+y, 1+z);
}

//called from the position below
inline void drawBottom(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    setBlock(b,l,0,tx,ty);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, z);
    glTexCoord2i(tx,ty);
    glVertex3i(x, 1+y, z);
    glTexCoord2i(tx,ty+1);
    glVertex3i(x, 1+y, 1+z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, 1+y, 1+z);
}

inline void drawFront(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    setBlock(b,l,3,tx,ty);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, 1+z);
    glTexCoord2i(tx,ty);
    glVertex3i(x, 1+y, 1+z);
    glTexCoord2i(tx,ty+1);
    glVertex3i(x, y, 1+z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, y, 1+z);
}

//called from the position behind
inline void drawBack(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    setBlock(b,l,2,tx,ty);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, 1+z);
    glTexCoord2i(tx,ty);
    glVertex3i(x, 1+y, 1+z);
    glTexCoord2i(tx,ty+1);
    glVertex3i(x, y, 1+z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, y, 1+z);
}

inline void drawRight(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    setBlock(b,l,5,tx,ty);
    glTexCoord2i(tx,ty);
    glVertex3i(1+x, 1+y, 1+z);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, y, z);
    glTexCoord2i(tx,ty+1);
    glVertex3i(1+x, y, 1+z);
}

//called from the position right
inline void drawLeft(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    setBlock(b,l,4,tx,ty);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, 1+z);
    glTexCoord2i(tx,ty);
    glVertex3i(1+x, 1+y, z);
    glTexCoord2i(tx,ty+1);
    glVertex3i(1+x, y, z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, y, 1+z);
}

inline void drawDecoration(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,b,6,tx,ty);
    glTexCoord2i(tx,ty+1);
    glVertex3i(x, y, z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(1+x, y, 1+z);
    glTexCoord2i(tx+1,ty);
    glVertex3i(1+x, 1+y, 1+z);
    glTexCoord2i(tx,ty);
    glVertex3i(x, 1+y, z);
    
    glTexCoord2i(tx,ty+1);
    glVertex3i(1+x, y, z);
    glTexCoord2i(tx+1,ty+1);
    glVertex3i(x, y, 1+z);
    glTexCoord2i(tx+1,ty);
    glVertex3i(x, 1+y, 1+z);
    glTexCoord2i(tx,ty);
    glVertex3i(1+x, 1+y, z);
}

inline void drawStaticChunk(Chunk *chunk, int cx, int cy, int cz, Chunk *ctop, Chunk *cbottom, Chunk *cright, Chunk *cleft, Chunk *cfront, Chunk *cback, std::map<Pos3D,Block*,Back2Front> &translucent) {
    int tx,ty,tz;
    worldPos(cx,cy,cz,tx,ty,tz);
    glTranslatef((float)tx,(float)ty,(float)tz);
    
    Block sky; //default constructor
    Block *blocks = chunk->blocks;
    
    glBegin(GL_QUADS);
    for (int x = 0; x < 16; x++) {
        Block *slice = &blocks[x*16*128];
        for (int z = 0; z < 16; z++) {
            Block *col = &slice[z*128];
            Block *ncol = &col[128];
            Block *nslicecol = &col[16*128];
            for (int y = 0; y < 128; y++) {
                switch (col[y].opacity()) {
                    case S_TRANSLUCENT:
                        translucent[Pos3D(x+tx,y+ty,z+tz)] = &col[y];
                    case S_AIR:
                        if (y < 127 && col[y+1].opacity() == S_OPAQUE) {
                            drawBottom(col[y+1],col[y],x,y,z);
                        }
                        if (x < 15 && nslicecol[y].opacity() == S_OPAQUE) { 
                            drawLeft(nslicecol[y],col[y],x,y,z);
                        }
                        if (z < 15 && ncol[y].opacity() == S_OPAQUE) {
                            drawBack(ncol[y],col[y],x,y,z);
                        }
                        break;
                    case S_OPAQUE:
                        if (y == 0) {
                            if (cbottom) {
                                drawBottom(col[0],cbottom->blocks[(x*16+z)*128+127],x,-1,z);
                            } else {
                                drawBottom(col[0],sky,x,-1,z);
                            }
                        }
                        if (y == 127) {
                            if (ctop) {
                                drawTop(col[127],ctop->blocks[(x*16+z)*128+0],x,127,z);
                            } else {
                                drawTop(col[127],sky,x,127,z);
                            }
                        } else if (col[y+1].opacity() != S_OPAQUE) {
                            drawTop(col[y],col[y+1],x,y,z);
                        }
                        
                        if (x == 0) {
                            if (cleft) {
                                drawLeft(col[y],cleft->blocks[(15*16+z)*128+y],-1,y,z);
                            } else {
                                drawLeft(col[y],sky,-1,y,z);
                            }
                        }
                        if (x == 15) {
                            if (cright) {
                                drawRight(col[y],cright->blocks[(0*16+z)*128+y],15,y,z);
                            } else {
                                drawRight(col[y],sky,15,y,z);
                            }
                        } else if (nslicecol[y].opacity() != S_OPAQUE) {
                            drawRight(col[y],nslicecol[y],x,y,z);
                        }
                        
                        if (z == 0) {
                            if (cback) {
                                drawBack(col[y],cback->blocks[(x*16+15)*128+y],x,y,-1);
                            } else {
                                drawBack(col[y],sky,x,y,-1);
                            }
                        }
                        if (z == 15) {
                            if (cfront) {
                                drawFront(col[y],cfront->blocks[(x*16+0)*128+y],x,y,15);
                            } else {
                                drawFront(col[y],sky,x,y,15);
                            }
                        } else if (ncol[y].opacity() != S_OPAQUE) {
                            drawFront(col[y],ncol[y],x,y,z);
                        }
                        break;
                }
            }
        }
    }
    glEnd();
    glTranslatef((float)-tx,(float)-ty,(float)-tz);
}

inline void drawTranslucentChunk(Chunk *chunk, int cx, int cy, int cz, Chunk *ctop, Chunk *cbottom, Chunk *cright, Chunk *cleft, Chunk *cfront, Chunk *cback, std::map<Pos3D,Block*,Back2Front> &translucent) {
    int tx,ty,tz;
    worldPos(cx,cy,cz,tx,ty,tz);
    glTranslatef((float)tx,(float)ty,(float)tz);
    
    Block sky; //default constructor
    Block *blocks = chunk->blocks;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
    std::map<Pos3D,Block*>::iterator i = translucent.begin();
    std::map<Pos3D,Block*>::iterator end = translucent.end();
    for ( ; i != end; i++) {
        int x,y,z; localPos(i->first.cx,i->first.cy,i->first.cz,x,y,z);
        Block *here = i->second;
        switch (here[0].style()) {
            case S_BLOCK:
                if (y == 127) {
                    if (ctop) {
                        if (ctop->blocks[(x*16+z)*128+0].opacity() < S_TRANSLUCENT)
                            drawTop(here[0],ctop->blocks[(x*16+z)*128+0],x,127,z);
                    } else {
                        drawTop(here[0],sky,x,127,z);
                    }
                } else if (here[1].opacity() < S_TRANSLUCENT) {
                    drawTop(here[0],here[1],x,y,z);
                }
                if (y == 0) {
                    if (cbottom) {
                        if (cbottom->blocks[(x*16+z)*128+127].opacity() < S_TRANSLUCENT) 
                            drawBottom(here[0],cbottom->blocks[(x*16+z)*128+127],x,-1,z);
                    } else {
                        drawBottom(here[0],sky,x,-1,z);
                    }
                } else if (here[-1].opacity() < S_TRANSLUCENT) {
                    drawBottom(here[0],here[-1],x,y-1,z);
                }
                if (x == 15) {
                    if (cright) {
                        if (cright->blocks[(0*16+z)*128+y].opacity() < S_TRANSLUCENT)
                            drawRight(here[0],cright->blocks[(0*16+z)*128+y],15,y,z);
                    } else {
                        drawRight(here[0],sky,15,y,z);
                    }
                } else if (here[128*16].opacity() < S_TRANSLUCENT) {
                    drawRight(here[0],here[128*16],x,y,z);
                }
                if (x == 0) {
                    if (cleft) { 
                        if (cleft->blocks[(15*16+z)*128+y].opacity() < S_TRANSLUCENT)
                            drawLeft(here[0],cleft->blocks[(15*16+z)*128+y],-1,y,z);
                    } else {
                        drawLeft(here[0],sky,-1,y,z);
                    }
                } else if (here[-128*16].opacity() < S_TRANSLUCENT) {
                    drawLeft(here[0],here[-128*16],x-1,y,z);
                }
                if (z == 15) {
                    if (cfront) {
                        if (cfront->blocks[(x*16+0)*128+y].opacity() < S_TRANSLUCENT)
                            drawFront(here[0],cfront->blocks[(x*16+0)*128+y],x,y,15);
                    } else {
                        drawFront(here[0],sky,x,y,15);
                    }
                } else if (here[128].opacity() < S_TRANSLUCENT) {
                    drawFront(here[0],here[128],x,y,z);
                }
                if (z == 0) {
                    if (cback) { 
                        if (cback->blocks[(x*16+15)*128+y].opacity() < S_TRANSLUCENT)
                            drawBack(here[0],cback->blocks[(x*16+15)*128+y],x,y,-1);
                    } else {
                        drawBack(here[0],sky,x,y,-1);
                    }
                } else if (here[-128].opacity() < S_TRANSLUCENT) {
                    drawBack(here[0],here[-128],x,y,z-1);
                }
                break;
            case S_DECORATION:
                drawDecoration(here[0],x,y,z);
                break;
            case S_SPECIAL:
                //doors and torches and levers and such
                break;
        }
    }
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    glTranslatef((float)-tx,(float)-ty,(float)-tz);
}

SDL_mutex *deaddatalock = SDL_CreateMutex();
std::vector<RenderData*> deaddata;
int times = 0;
int num = 0;

void renderWorld(Client *client) {

    SDL_mutexP(deaddatalock);
    while (!deaddata.empty()) {
        RenderData *old = deaddata.back();
        if (old->haslist) glDeleteLists(old->list, 2);
        if (old->translucent) delete old->translucent;
        delete old;
        deaddata.pop_back();
    }
    SDL_mutexV(deaddatalock);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double fx = cos(client->us->pitch/180.0*3.14159)*cos((client->us->yaw)/180.0*3.14159);
    double fz = cos(client->us->pitch/180.0*3.14159)*sin((client->us->yaw)/180.0*3.14159);
    double fy = sin(client->us->pitch/180.0*3.14159);
    glRotatef(client->us->pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(client->us->yaw+90.0f, 0.0f, 1.0f, 0.0f);
    
    double px = client->us->x;
    double py = client->us->y+client->us->height;
    double pz = client->us->z;
    glTranslatef(-px, -py, -pz); 

    int time = SDL_GetTicks();
    
    std::map<Pos3D,Chunk*,Back2Front> visible(Back2Front(px/16.0-0.5,py/128.0-0.5,pz/16.0-0.5));
    std::map<Pos3D,Block*,Back2Front> translucent(Back2Front(px,py,pz));
    
    client->world.lockChunks();
    std::map<Pos3D,Chunk*>::iterator ci = client->world.chunks.begin();
    std::map<Pos3D,Chunk*>::iterator end = client->world.chunks.end();
    for ( ; ci != end; ci++) {
        int cx = ci->first.cx;
        int cy = ci->first.cy;
        int cz = ci->first.cz;
        int wx,wy,wz;
        worldPos(cx,cy,cz,wx,wy,wz);
        wx += 8 - px;
        wz += 8 - pz;
        double len  = sqrt(wx*wx+wz*wz);
        double angle = acos((wx*fx+wz*fz)/len);
        //only render IF the chunk center is less than 40 blocks from us or in the 180 degree FOV in front of us
        if (len < 20 || abs(angle) <= 90.0/180.0*3.14159) {
            Chunk *chunk = ci->second;
            visible[Pos3D(cx,cy,cz)] = chunk;
            Chunk *ctop = client->world.getChunkIdx(cx,cy+1,cz),
                  *cbottom = client->world.getChunkIdx(cx,cy-1,cz),
                  *cright = client->world.getChunkIdx(cx+1,cy,cz),
                  *cleft = client->world.getChunkIdx(cx-1,cy,cz),
                  *cfront = client->world.getChunkIdx(cx,cy,cz+1),
                  *cback = client->world.getChunkIdx(cx,cy,cz-1);
            RenderData *data = (RenderData*)chunk->renderdata;
            if (!data) {
                chunk->destroy = &disposeChunk;
                chunk->renderdata = data = new RenderData;
                data->haslist = true;
                data->list = glGenLists(2);
                data->translucent = new std::map<Pos3D,Block*,Back2Front>(Back2Front(px,py,pz));
                num++;
            }
            if (chunk->boundarydirty || chunk->dirty || !chunk->renderdata) {
                chunk->boundarydirty = false;
                if (chunk->dirty) {
                    if (ctop) ctop->markBoundaryDirty();
                    if (cbottom) cbottom->markBoundaryDirty();
                    if (cright) cright->markBoundaryDirty();
                    if (cleft) cleft->markBoundaryDirty();
                    if (cfront) cfront->markBoundaryDirty();
                    if (cback) cback->markBoundaryDirty();
                }
                chunk->dirty = false;
                data->translucent->clear();
                glNewList(data->list, GL_COMPILE);
                drawStaticChunk(chunk,cx,cy,cz,ctop,cbottom,cright,cleft,cfront,cback,*data->translucent);
                glEndList();
                glNewList(data->list+1, GL_COMPILE);
                drawTranslucentChunk(chunk,cx,cy,cz,ctop,cbottom,cright,cleft,cfront,cback,*data->translucent);
                glEndList();
            } else if (len < 20) {
                std::map<Pos3D,Block*,Back2Front> *sorted = new std::map<Pos3D,Block*,Back2Front>(data->translucent->begin(),data->translucent->end(),Back2Front(px,py,pz));
                delete data->translucent;
                data->translucent = sorted;
                glNewList(data->list+1, GL_COMPILE);
                drawTranslucentChunk(chunk,cx,cy,cz,ctop,cbottom,cright,cleft,cfront,cback,*data->translucent);
                glEndList();
            }
            glCallList(data->list);
        }
    }
    std::map<Pos3D,Chunk*,Back2Front>::iterator vi = visible.begin();
    std::map<Pos3D,Chunk*,Back2Front>::iterator vend = visible.end();
    for ( ; vi != vend; vi++) {
        glCallList(((RenderData*)vi->second->renderdata)->list+1);
    }
    client->world.unlockChunks();
    time = SDL_GetTicks()-time;

    glFlush(); 
    SDL_GL_SwapBuffers();
    
    if (!(times++ % 30)) std::cout << "Rendered in " << std::dec << time << " ms (" << (float)time/visible.size() << " per chunk)\n";
}

void disposeChunk(Chunk *chunk) {
    SDL_mutexP(deaddatalock);
    if (chunk->renderdata) deaddata.push_back((RenderData*)chunk->renderdata);
    SDL_mutexV(deaddatalock);
}

bool capture_mouse = true;

void processEvents(Client *client) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        capture_mouse = !capture_mouse;
                        SDL_ShowCursor(capture_mouse ? SDL_DISABLE : 1);
                        SDL_WM_GrabInput(capture_mouse ? SDL_GRAB_ON : SDL_GRAB_OFF);
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                if (capture_mouse) client->relLook(event.motion.yrel/2.0,event.motion.xrel/2.0);
                break;
            case SDL_QUIT:
                client->disconnect();
        }
    }
    unsigned char *keystate = SDL_GetKeyState(NULL);
    if (keystate[SDLK_SPACE]) client->jump();
    double forwards = 5.0*(keystate[SDLK_w] ? 1 : keystate[SDLK_s] ? -1 : 0);
    double sideways = 5.0*(keystate[SDLK_d] ? 1 : keystate[SDLK_a] ? -1 : 0);
    client->setMotion(forwards,sideways);
}

void quitRender() {
    SDL_Quit();
}
