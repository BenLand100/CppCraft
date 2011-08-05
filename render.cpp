#include "render.h"
#include "world.h"
#include <iostream>
#include <cmath>
#include <map>
#include <SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>


#define w_width 800
#define w_height 600

float intensity[16];

GLvoid initGL(GLsizei width, GLsizei height) {
    glViewport(0,0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(100.0f, (GLfloat)width / (GLfloat)height, 0.1f, 500.0f);

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
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_EMISSION);
    for (int i = 0; i < 16; i++) {
        intensity[15-i] = pow(0.8,i);
    }
    
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
    initGL(w_width, w_height);
    return true;
}

inline void setBlock(Block &block, int face) {
    float r,g,b;
    switch (block.type) {
        case 1:
            r = 0.3f; g = 0.3f; b = 0.3f; break;
        case 2:
            r = 0.1f; g = 0.9f; b = 0.1f; break;
        case 3:
            r = 0.7f; g = 0.5f; b = 0.3f; break;
        case 4:
            r = 0.5f; g = 0.5f; b = 0.5f; break;
        default:
            r = 1.0f; g = 0.0f; b = 0.0f; break;
            
    }
    float f = intensity[block.sky];
    glColor3f(r*f,g*f,b*f);
}

inline void drawTop(Block &b, int x, int y, int z) {
    setBlock(b,0);
    glBegin(GL_QUADS);
        glNormal3i(0,1,0);
        glVertex3i(1+x, 1+y, z);
        glVertex3i(x, 1+y, z);
        glVertex3i(x, 1+y, 1+z);
        glVertex3i(1+x, 1+y, 1+z);
    glEnd();
}

//called from the position below
inline void drawBottom(Block &b, int x, int y, int z) {
    setBlock(b,1);
    glBegin(GL_QUADS);
        glNormal3i(0,-1,0);
        glVertex3i(1+x, 1+y, z);
        glVertex3i(x, 1+y, z);
        glVertex3i(x, 1+y, 1+z);
        glVertex3i(1+x, 1+y, 1+z);
    glEnd();
}

inline void drawFront(Block &b, int x, int y, int z) {
    setBlock(b,2);
    glBegin(GL_QUADS);
        glNormal3i(0,0,1);
        glVertex3i(1+x, 1+y, 1+z);
        glVertex3i(x, 1+y, 1+z);
        glVertex3i(x, y, 1+z);
        glVertex3i(1+x, y, 1+z);
    glEnd();
}

//called from the position behind
inline void drawBack(Block &b, int x, int y, int z) {
    setBlock(b,3);
    glBegin(GL_QUADS);
        glNormal3i(0,0,-1);
        glVertex3i(1+x, 1+y, 1+z);
        glVertex3i(x, 1+y, 1+z);
        glVertex3i(x, y, 1+z);
        glVertex3i(1+x, y, 1+z);
    glEnd();
}

inline void drawRight(Block &b, int x, int y, int z) {
    setBlock(b,4);
    glBegin(GL_QUADS);
        glNormal3i(1,0,0);
        glVertex3f(1+x, 1+y, 1+z);
        glVertex3f(1+x, 1+y, z);
        glVertex3f(1+x, y, z);
        glVertex3f(1+x, y, 1+z);
    glEnd();
}

//called from the position right
inline void drawLeft(Block &b, int x, int y, int z) {
    setBlock(b,5);
    glBegin(GL_QUADS);
        glNormal3i(-1,0,0);
        glVertex3f(1+x, 1+y, 1+z);
        glVertex3f(1+x, 1+y, z);
        glVertex3f(1+x, y, z);
        glVertex3f(1+x, y, 1+z);
    glEnd();
}

inline void drawChunk(Chunk *chunk, int cx, int cy, int cz) {
    int buffer[] = {1,1,0,0,1,0,0,1,1,1,1,1};
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_INT, 0, buffer);
    int tx,ty,tz;
    worldPos(cx,cy,cz,tx,ty,tz);
    glTranslatef((float)tx,(float)ty,(float)tz);
    for (int x = 0; x < 16; x++) {
        Block *slice = &chunk->blocks[x*16*128];
        for (int z = 0; z < 16; z++) {
            Block *col = &slice[z*128];
            Block *ncol = &col[128];
            Block *nslicecol = &col[16*128];
            for (int y = 0; y < 128; y++) {
                if (!col[y].type) {  //SHOULD CHECK TRANSPARENCY 
                    if (y < 127 && col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawBottom(col[y+1],x,y,z);
                    }
                    if (x < 15 && nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawLeft(nslicecol[y],x,y,z);
                    }
                    if (z < 15 && ncol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawBack(ncol[y],x,y,z);
                    }
                } else {
                    if (y == 127 || !col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawTop(col[y],x,y,z);
                    }
                    if (x == 15 || !nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawRight(col[y],x,y,z);
                    }
                    if (z == 15 || !ncol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawFront(col[y],x,y,z);
                    }
                }
            }
        }
    }
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
        int wx,wy,wz;
        worldPos(cx,cy,cz,wx,wy,wz);
        wx -= px;
        wy -= py;
        wz -= pz;
        if (wx*wx+wy*wy+wz*wz <= 300*300)
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
