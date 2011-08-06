#ifndef _render
#define _render

#include "client.h"

bool initRender();
void renderWorld(Client *client);
void processEvents(Client *client);
void quitRender();
void disposeChunk(Chunk *chunk);

#endif
