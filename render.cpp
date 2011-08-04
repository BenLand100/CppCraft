#include "render.h"
#include "world.h"
#include <iostream>
#include <map>
#include <SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>


#define w_width 400
#define w_height 300

GLvoid initGL(GLsizei width, GLsizei height) {
    glViewport(0,0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(100.0f, (GLfloat)width / (GLfloat)height, 0.1f, 500.0f);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_PERSPECTIVE_CORRECTION_HINT);
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
    initGL(w_width, w_height);
    return true;
}

inline void setBlock(Block &b, int face) {
    switch (b.type) {
        case 1:
            glColor3f(0.3f,0.3f,0.3f);
            break;
        case 2:
            glColor3f(0.1f,0.9f,0.1f);
            break;
        case 3:
            glColor3f(0.7f,0.5f,0.3f);
            break;
        case 4:
            glColor3f(0.5f,0.5f,0.5f);
            break;
        default:
            glColor3f(1.0f,0,0);
            
    }
}

inline void drawTop(Block &b) {
    setBlock(b,0);
    glBegin(GL_QUADS);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();
}

//called from the position below
inline void drawBottom(Block &b) {
    setBlock(b,1);
    glBegin(GL_QUADS);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();
}

inline void drawFront(Block &b) {
    setBlock(b,2);
    glBegin(GL_QUADS);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(0.0f, 1.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();
}

//called from the position behind
inline void drawBack(Block &b) {
    setBlock(b,3);
    glBegin(GL_QUADS);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(0.0f, 1.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();
}

inline void drawRight(Block &b) {
    setBlock(b,5);
    glBegin(GL_QUADS);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();
}

//called from the position right
inline void drawLeft(Block &b) {
    setBlock(b,4);
    glBegin(GL_QUADS);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();
}

inline void drawChunk(Chunk *chunk, int cx, int cy, int cz) {
    int tx,ty,tz;
    worldPos(cx,cy,cz,tx,ty,tz);
    glTranslatef((float)tx,(float)ty,(float)tz);
    for (int x = 0; x < 16; x++) {
        Block *slice = &chunk->blocks[x*16*128];
        glTranslatef(1.0f, 0, 0);
        for (int z = 0; z < 16; z++) {
            Block *col = &slice[z*128];
            Block *ncol = &col[128];
            Block *nslicecol = &col[16*128];
            glTranslatef(0, 0, 1.0f);
            for (int y = 0; y < 128; y++) {
                glTranslatef(0, 1.0f, 0);
                if (!col[y].type) {  //SHOULD CHECK TRANSPARENCY 
                    if (y < 127 && col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawBottom(col[y+1]);
                    }
                    if (x < 15 && nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawLeft(nslicecol[y]);
                    }
                    if (z < 15 && ncol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawBack(ncol[y]);
                    }
                } else {
                    if (y == 127 || !col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawTop(col[y]);
                    }
                    if (x == 15 || !nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawRight(col[y]);
                    }
                    if (z == 15 || !ncol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawFront(col[y]);
                    }
                }
            }
            glTranslatef(0, -128.0f, 0);
        }
        glTranslatef(0, 0, -16.0f);
    }
    glTranslatef(-16.0f, 0, 0);
    glTranslatef((float)-tx,(float)-ty,(float)-tz);
}

float yRotationAngle = 0;

void renderWorld(Client *client) {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotatef(yRotationAngle+=1, 0.0f, 1.0f, 0.0f);//should do pitch/yaw here, now it's just a look around
    double px = client->us->x;
    double py = client->us->y;
    double pz = client->us->z;
    glTranslatef(-px, -(py+client->us->height), -pz); 

    int time = SDL_GetTicks();
    
    client->world.lock();
    std::map<ChunkPos,Chunk*>::iterator ci = client->world.chunks.begin();
    std::map<ChunkPos,Chunk*>::iterator end = client->world.chunks.end();
    int i = 0;
    for ( ; ci != end; ci++, i++) {
        int cx = ci->first.cx;
        int cy = ci->first.cy;
        int cz = ci->first.cz;
        /*int wx,wy,wz;
        worldPos(cx,cy,cz,wx,wy,wz);
        wx -= px;
        wy -= py;
        wz -= pz;*/
        drawChunk(ci->second,cx,cy,cz);
    }
    client->world.unlock();
    
    time = SDL_GetTicks()-time;

    glFlush(); 
    SDL_GL_SwapBuffers();
    
    std::cout << "Rendered in " << std::dec << time << " ms (" << (float)time/i << " per chunk)\n";
}

void quitRender() {
    SDL_Quit();
}
