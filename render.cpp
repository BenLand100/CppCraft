#include "render.h"
#include "world.h"
#include <iostream>
#include <cmath>
#include <map>
#include <SDL.h>
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
    initGL(w_width, w_height);
    return true;
}

//Face {0,1,2,3,4,5} == {-y,+y,-z,+z,-x,+x}
inline int setBlock(Block &block, int face, int &tx, int &ty) {
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
        default:
            tx = 0; ty = 0; g = 0.0f; b = 0.0f;
            
    }
    int light = block.light + block.sky;
    float f = intensity[light < 16 ? light : 15];
    glColor3f(r*f,g*f,b*f);
}

inline void drawTop(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,1,tx,ty);
    glBegin(GL_QUADS);
        glNormal3i(0,1,0);
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, 1+y, 1+z);
    glEnd();
}

//called from the position below
inline void drawBottom(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,0,tx,ty);
    glBegin(GL_QUADS);
        glNormal3i(0,-1,0);
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, 1+y, 1+z);
    glEnd();
}

inline void drawFront(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,3,tx,ty);
    glBegin(GL_QUADS);
        glNormal3i(0,0,1);
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, 1+z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, y, 1+z);
    glEnd();
}

//called from the position behind
inline void drawBack(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,2,tx,ty);
    glBegin(GL_QUADS);
        glNormal3i(0,0,-1);
        glTexCoord2i(tx+1,ty);
        glVertex3i(1+x, 1+y, 1+z);
        glTexCoord2i(tx,ty);
        glVertex3i(x, 1+y, 1+z);
        glTexCoord2i(tx,ty+1);
        glVertex3i(x, y, 1+z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3i(1+x, y, 1+z);
    glEnd();
}

inline void drawRight(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,5,tx,ty);
    glBegin(GL_QUADS);
        glNormal3i(1,0,0);
        glTexCoord2i(tx,ty);
        glVertex3f(1+x, 1+y, 1+z);
        glTexCoord2i(tx+1,ty);
        glVertex3f(1+x, 1+y, z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3f(1+x, y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3f(1+x, y, 1+z);
    glEnd();
}

//called from the position right
inline void drawLeft(Block &b, int x, int y, int z) {
    int tx,ty;
    setBlock(b,4,tx,ty);
    glBegin(GL_QUADS);
        glNormal3i(-1,0,0);
        glTexCoord2i(tx+1,ty);
        glVertex3f(1+x, 1+y, 1+z);
        glTexCoord2i(tx,ty);
        glVertex3f(1+x, 1+y, z);
        glTexCoord2i(tx,ty+1);
        glVertex3f(1+x, y, z);
        glTexCoord2i(tx+1,ty+1);
        glVertex3f(1+x, y, 1+z);
    glEnd();
}

inline void drawChunk(Chunk *chunk, int cx, int cy, int cz) {
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
                    if (y == 0) {
                        drawBottom(col[y],x,y-1,z);
                    }
                    if (y == 127 || !col[y+1].type) { //SHOULD CHECK TRANSPARENCY
                        drawTop(col[y],x,y,z);
                    }
                    if (x == 0) {
                        drawLeft(col[y],x-1,y,z);
                    }
                    if (x == 15 || !nslicecol[y].type) { //SHOULD CHECK TRANSPARENCY
                        drawRight(col[y],x,y,z);
                    }
                    if (z == 0) {
                        drawBack(col[y],x,y,z-1);
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
