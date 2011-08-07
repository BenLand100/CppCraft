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
    
    /*GLfloat LightAmbient[]= { 0.5f, 0.5f, 0.5f, 0.75f }; 
    GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 0.75f };
    GLfloat LightSpecular[]= { 1.0f, 1.0f, 1.0f, 0.75f };
    GLfloat LightPosition[]= { 0,0,0, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse); 
    glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular); 
    glLightfv(GL_LIGHT0, GL_POSITION,LightPosition); 
    glEnable(GL_LIGHT0); */
    
    glEnable(GL_LIGHTING);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
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
    
    /*float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
    glMateriali(GL_FRONT, GL_SHININESS, 128);*/
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
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);
    initGL(w_width, w_height);
    return true;
}

//Face {0,1,2,3,4,5} == {-y,+y,-z,+z,-x,+x}
//Result indicates static block if true (used in make display lists)
inline bool setBlock(Block &block, Block &l, int face, int &tx, int &ty) {
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
            
        default:
            tx = 0; ty = 0; g = 0.0f; b = 0.0f;
            
    }
    float f = intensity[l.sky < 16 ? l.sky : 15];
    switch (face) {
        case 2: case 3:
            f *= 0.8; break;
        case 4: case 5:
            f *= 0.6; break;
    }
    glColor3f(r*f,g*f,b*f);
    return true;
}

inline void drawTop(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    if (setBlock(b,l,1,tx,ty)) {
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, 1+y, 1+z);
    }
}

//called from the position below
inline void drawBottom(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    if (setBlock(b,l,0,tx,ty)) {
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, 1+y, 1+z);
    }
}

inline void drawFront(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    if (setBlock(b,l,3,tx,ty)) {
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, 1+z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, y, 1+z);
    }
}

//called from the position behind
inline void drawBack(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    if (setBlock(b,l,2,tx,ty)) {
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, 1+z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, y, 1+z);
    }
}

inline void drawRight(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    if (setBlock(b,l,5,tx,ty)) {
        glTexCoord2i(tx,ty);
        glVertex3i(1+x, 1+y, 1+z);
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(1+x, y, 1+z);
    }
}

//called from the position right
inline void drawLeft(Block &b, Block &l, int x, int y, int z) {
    int tx,ty;
    if (setBlock(b,l,4,tx,ty)) {
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, 1+z);
        glTexCoord2i(tx,ty);
        glVertex3i(1+x, 1+y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(1+x, y, z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, y, 1+z);
    }
}


//These are used for back-face culling (not implemented atm) because it would actually be slower
#define TOP (y < py)
#define RIGHT (x < px)
#define FRONT (z < pz)
#define BOTTOM (y+2 > py)
#define LEFT (x+2 > px)
#define BACK (z+2 > pz)

inline void drawStaticChunk(Chunk *chunk, int cx, int cy, int cz, int px, int py, int pz) {
    int tx,ty,tz;
    worldPos(cx,cy,cz,tx,ty,tz);
    glTranslatef((float)tx,(float)ty,(float)tz);
    px -= tx;
    py -= ty;
    pz -= tz;
    
    Block sky; //default constructor
    Block *blocks = chunk->blocks;
    
    glBegin(GL_QUADS);
    int x = 0;
    for (int z = 0; z < 16; z++) {
        for (int y = 0; y < 128; y++) {
            if (blocks[z*128+y].type) drawLeft(blocks[z*128+y],sky,-1,y,z); //SHOULD CHECK TRANSPARENCY
        }
    }
    for (x = 0; x < 16; x++) {
        Block *slice = &blocks[x*16*128];
        int z = 0;
        for (int y = 0; y < 128; y++) {
            if (slice[y].type) drawBack(slice[y],sky,x,y,-1); //SHOULD CHECK TRANSPARENCY
        }
        for (z = 0; z < 16; z++) {
            Block *col = &slice[z*128];
            Block *ncol = &col[128];
            Block *nslicecol = &col[16*128];
            int y = 0;
            if (col[0].type) drawBottom(col[0],sky,x,-1,z); //SHOULD CHECK TRANSPARENCY
            for (y = 0; y < 128; y++) {
                if (!col[y].type) {  //SHOULD CHECK TRANSPARENCY 
                    if (y < 127 && col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawBottom(col[y+1],col[y],x,y,z);
                    }
                    if (x < 15 && nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawLeft(nslicecol[y],col[y],x,y,z);
                    }
                    if (z < 15 && ncol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawBack(ncol[y],col[y],x,y,z);
                    }
                } else {
                    if (y == 127) {
                        drawTop(col[y],sky,x,y,z);
                    } else if (!col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawTop(col[y],sky,x,y,z);
                    }
                    if (x == 15) {
                        drawRight(col[y],sky,x,y,z);
                    } else if (!nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawRight(col[y],nslicecol[y],x,y,z);
                    }
                    if (z == 15) {
                        drawFront(col[y],sky,x,y,z);
                    } else if (!ncol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawFront(col[y],ncol[y],x,y,z);
                    }
                }
            }
        }
    }
    glEnd();
    
    glTranslatef((float)-tx,(float)-ty,(float)-tz);
}

SDL_mutex *listlock = SDL_CreateMutex();
std::vector<int> lists;
int times = 0;

void renderWorld(Client *client) {

    SDL_mutexP(listlock);
    while (!lists.empty()) {
        glDeleteLists(lists.back(), 1);
        lists.pop_back();
    }
    SDL_mutexV(listlock);
    
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
    
    client->world.lock();
    std::map<ChunkPos,Chunk*>::iterator ci = client->world.chunks.begin();
    std::map<ChunkPos,Chunk*>::iterator end = client->world.chunks.end();
    int i = 0;
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
        if (len < 40 || abs(angle) <= 90.0/180.0*3.14159) { 
            i++;
            Chunk *chunk = ci->second;
            if (chunk->dirty || !chunk->haslist) {
                chunk->dirty = false;
                if (!chunk->haslist) {
                    chunk->haslist = true;
                    chunk->list = glGenLists(1);
                }
                glNewList(chunk->list, GL_COMPILE);
                drawStaticChunk(chunk,cx,cy,cz,px,py,pz);
                glEndList();
            }
            glCallList(chunk->list);
        }
    }
    client->world.unlock();
    time = SDL_GetTicks()-time;

    glFlush(); 
    SDL_GL_SwapBuffers();
    
    if (!(times++ % 30)) std::cout << "Rendered in " << std::dec << time << " ms (" << (float)time/i << " per chunk)\n";
}

void disposeChunk(Chunk *chunk) {
    if (chunk->haslist) {
        SDL_mutexP(listlock);
        lists.push_back(chunk->list);
        SDL_mutexV(listlock);
    }
}

bool capture_mouse = true;

void processEvents(Client *client) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        client->jump();
                        break;
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
    double forwards = 5.0*(keystate[SDLK_w] ? 1 : keystate[SDLK_s] ? -1 : 0);
    double sideways = 5.0*(keystate[SDLK_d] ? 1 : keystate[SDLK_a] ? -1 : 0);
    client->setMotion(forwards,sideways);
}

void quitRender() {
    SDL_Quit();
}
