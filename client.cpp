#include <iostream>
#include "client.h"
#include "packets.h"
#include "render.h"
#include <cmath>

Client::Client() {
    ax = ay = az = 0.0;
    connected = false;
    socket = NULL;
    doPhysics = false;
    physics = NULL;
    doPackets = false;
    packets = NULL;
    us = NULL;
    usLock = false;
    usLock = SDL_CreateMutex();
}

Client::~Client() {
    disconnect();
    SDL_DestroyMutex(usLock);
}

void Client::lockUs() {
    SDL_mutexP(usLock);
}

void Client::unlockUs() {
    SDL_mutexV(usLock);
}
        
bool Client::connect(char *host, int port) {
    if (!socket) {
        IPaddress ip;
        if (SDLNet_ResolveHost(&ip, host, port) < 0) {
	        std::cout << "Error resolving host: " << SDLNet_GetError() << '\n';
	        return false;
        }
        if (!(socket = SDLNet_TCP_Open(&ip))) {
	        std::cout << "Error connecting: " << SDLNet_GetError() << '\n';
	        socket = NULL;
	        return false;
        }
        connected = true;
        doPackets = true;
        packets = SDL_CreateThread((int (*)(void*))packets_thread, this);
        return true;
    } else {
        return false;
    }
}

int physics_thread(Client *client) {
    //for some reason these get random values...
    client->us->pitch = 0;
    client->us->yaw = 0;
    while (client->doPhysics) {
        SDL_Delay(50);
        client->lockUs();
        Block *feet = client->world.getBlock(client->us->x,client->us->y-0.05,client->us->z);
        if (feet) switch (feet->type) {
            case 0:
                client->onGround = false;
                break;
        }
        if (!client->onGround) {
            if (feet) switch (feet->type) {
                case 0:
                    client->us->vy -= 9.8*0.05;
                    break;
                default:
                    client->us->y = floor(client->us->y);
                    client->us->vy = 0.0;
                    client->onGround = true;
            }
        }
        
        client->us->vx += client->ax*0.05;
        client->us->vy += client->ay*0.05;
        client->us->vz += client->az*0.05;
        
        if (client->us->vx > 5.0) client->us->vx = 5.0;
        if (client->us->vy > 10.0) client->us->vy = 5.0;
        if (client->us->vz > 5.0) client->us->vz = 5.0;
        
        client->us->x += client->us->vx*0.05;
        client->us->y += client->us->vy*0.05;
        client->us->z += client->us->vz*0.05;
        
        client->sendPos();
        client->unlockUs();
    }
    return 0;
}

void Client::packet(p_generic *p) {
    switch (p->id) {
        case 0x00:
            send_keep_alive(socket);
            break;
        case 0x01:
            std::cout << "Logged In! EID: " << std::dec <<((p_login_request_stc*)p)->EntityID << '\n';
            break;
        case 0x02:
            send_login_request_cts(socket,14,us->name,0,0);
            break;
        case 0x03:
            std::cout << "Chat: " << ((p_chat_message*)p)->Message << '\n';
            break;
        case 0x0D:
            {
                p_player_position_and_look_stc *pos = (p_player_position_and_look_stc*)p;
                lockUs();
                onGround = pos->OnGround;
                us->x = pos->X;
                us->y = pos->Y;
                us->z = pos->Z;
                us->height = pos->Stance - pos->Y;
                us->pitch = pos->Yaw;
                us->yaw = pos->Pitch;
                sendPos();
                unlockUs();
            }
            if (!physics) {
                doPhysics = true;
                physics = SDL_CreateThread((int (*)(void*))physics_thread, this);
            }
            break;
        case 0x18:
            break;
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x26:
            break; //ignore entity motion stuff for now
        case 0x32: 
            {
                p_prechunk *prechunk = (p_prechunk*)p;
                if (!prechunk->Mode) world.deleteChunk(prechunk->X,0,prechunk->Z);
            }
            break;
        case 0x33:
            {
                p_map_chunk *update = (p_map_chunk*)p;
                bool res = world.updateChunk(update->X,update->Y,update->Z,update->SizeX,update->SizeY,update->SizeZ,update->CompressedSize,update->CompressedData);
                if (!res) {
                    std::cout << "Error performing chunk update\n";
                    disconnect();
                }
            }
            break;
        case 0x34:
            {
                p_multi_block_change *mbc = (p_multi_block_change*)p;
                bool res = world.updateChunk(mbc->ChunkX,0,mbc->ChunkZ,mbc->ArraySize,mbc->CoordinateArray,mbc->TypeArray,mbc->MetadataArray);
                if (!res) {
                    std::cout << "Error performing multi block change\n";
                    //disconnect();
                }
            }
            break;
        case 0x35:
            {
                p_block_change *change = (p_block_change*)p;
                Block *b = world.getBlock(change->X,change->Y,change->Z);
                if (b) {
                    b->type = change->Type;
                    b->meta = change->Metadata;
                    world.getChunk(change->X,change->Y,change->Z)->markDirty();
                } else {
                    std::cout << "Changed a block in an undefined chunk?\n";
                }
            }
            break;
        case 0xFF:
            std::cout << "KICK: " << ((p_kick*)p)->Message << '\n';
            disconnect();
            break;
        default:
            std::cout << "Unhandled Packet: 0x" << std::hex << (int)p->id << '\n';
    }
}

void Client::disconnect() {
    doPackets = false;
    doPhysics = false;
    if (physics) SDL_WaitThread(physics, NULL);
    if (packets) SDL_WaitThread(packets, NULL);
    physics = NULL;
    packets = NULL;
    connected = false;
    if (socket) SDLNet_TCP_Close(socket);
    socket = NULL;
    if (us) delete us;
    us = NULL;
    world.clearChunks();
}

bool Client::login(char *username) {
    if (!us) {
        us = new Player(username);
        send_handshake_cts(socket,us->name);
        return true;
    }
}

bool Client::running() {
    return connected;
}

void Client::sendPos() {
    send_player_position_and_look_cts(socket,us->x,us->y,us->height+us->y,us->z,us->yaw,us->pitch,onGround);
}

void Client::init() {
    if (SDLNet_Init() < 0) {
        std::cout << "Failed to init SDLNet: " << SDLNet_GetError() <<'\n';
        exit(1);
    }
}

void Client::quit() {
    SDLNet_Quit();
}

int main(int argc, char** argv) {

    Client::init();
    initRender();
    Client *c = new Client();
    if (c->connect((char*)"localhost")) {
        if (c->login((char*)"YourMom")) {
            while (c->running()) {
                SDL_Delay(10);
                renderWorld(c);
            }
        }
    }
    delete c;
    quitRender();
    Client::quit();
    
    std::cout << "Finished!\n";
    
    return 0;
}
