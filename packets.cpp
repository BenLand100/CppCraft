#include <iostream>
#include "packets.h"
#include "SDL_endian.h"

class SocketIO {
    public:
        bool working;
    private:
        TCPsocket socket;
        unsigned char buffer[16];
        void read(int len) {
            if (SDLNet_TCP_Recv(socket, buffer, len) != len) {
                working = false;
                for (int i = 0; i < 16; i++) buffer[i] = 0;
            }
        }
        
    public:
        SocketIO(TCPsocket _socket) : socket(_socket), working(true) { }
        unsigned char r_ubyte() {
            read(1);
            return buffer[0];
        }
        char r_byte() {
            read(1);
            return (char) buffer[0];
        }
        short r_short() {
            read(2);
            return SDL_SwapBE16(*(Uint16*)buffer);
        }
        int r_int() {
            read(4);
            return SDL_SwapBE32(*(Uint32*)buffer);
        }
        long long r_long() {
            read(8);
            return SDL_SwapBE64(*(Uint64*)buffer);
        }
        float r_float() {
            read(4);
            union { float f; Uint32 i; } swap;
            swap.i = SDL_Swap32(*(Uint32*)buffer);
            return swap.f;
        }
        double r_double() {
            read(8);
            union { double d; Uint64 l; } swap;
            swap.l = SDL_Swap64(*(Uint64*)buffer);
            return swap.d;
        }
        char* r_string8() {
            int len = r_short();
            char *res = new char[len];
            if (SDLNet_TCP_Recv(socket, res, len) != len) {
                working = false;
            }
        }
        char* r_string16() {
            int len = r_short()*2;
            char *res = new char[len];
            if (SDLNet_TCP_Recv(socket, res, len) != len) {
                working = false;
            }
        }
        bool r_bool() {
            read(1);
            return buffer[0];
        }



};

int packets_thread(Client *client) {
    SocketIO io(client->socket);
    while (client->doPackets && io.working) {
        unsigned char pid = io.r_ubyte();
        switch (pid) {
        
            default:
                std::cout << "Unhandled packet: 0x" << std::hex << (int)pid << '\n';
                client->disconnect();
                return 1;
        }
    }
    if (!io.working) {
        std::cout << "Socket Error\n";
        client->disconnect();
    }
    return 0;
}

