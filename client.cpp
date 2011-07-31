#include <iostream>
#include "client.h"
#include "packets.h"

Client::Client() {
    connected = false;
    socket = NULL;
    doPhysics = false;
    physics = NULL;
    doPackets = false;
    packets = NULL;
}

Client::~Client() {
    doPackets = false;
    doPhysics = false;
    if (physics) SDL_WaitThread(physics, NULL);
    if (packets) SDL_WaitThread(packets, NULL);
    disconnect();
}
        
bool Client::connect(char *host, int port) {
    if (!socket) {
        IPaddress ip;
        if (SDLNet_ResolveHost(&ip, host, port) < 0) {
	        std::cout << "Could not resolve host: " << SDLNet_GetError() << '\n';
	        return false;
        }
        if (!(socket = SDLNet_TCP_Open(&ip))) {
	        std::cout << "Could not connect to host: " << SDLNet_GetError() << '\n';
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

void Client::disconnect() {
    connected = false;
    if (socket) SDLNet_TCP_Close(socket);
}

bool Client::login(char *username) {
}

bool Client::running() {
    return connected;
}

void Client::sendPos() {
}


int physics_thread(Client *client) {
    while (client->doPhysics) {
        //lolphysics
        client->sendPos();
    }
    return 0;
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

    Client *c = new Client();
    if (c->connect((char*)"localhost") && c->login((char*)"YourMom")) {
        std::cout << "Client initilized!\n";
        while (c->running()) {
            SDL_Delay(1000);
        }
        std::cout << "Finished!\n";
    } else {
        std::cout << "Error connecting to server!\n";
    }
    
    delete c;
    
    Client::quit();
    return 0;
}
