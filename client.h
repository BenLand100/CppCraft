#ifndef _client
#define _client

#include "SDL_thread.h"
#include "SDL_net.h"
#include "packets.h"
#include "entities.h"
#include "world.h"

class Client {
    public:
        static void init();
        static void quit();
    
        Client();
        virtual ~Client();
        
        bool connect(char *host, int port = 25565);
        void disconnect();
        bool login(char *username);
        
        void jump();
        void relLook(double dpitch, double dyaw);
        void setMotion(double forwards, double sideways);
        
        bool running();
        
        void lockUs();
        void unlockUs();
        
    private:
        double ax,ay,az;
        World world;
        Player *us;
        bool onGround;
        bool connected;
        TCPsocket socket;
        bool doPhysics,doPackets;
        SDL_Thread *physics,*packets;
        SDL_mutex *usLock;
        
        void packet(p_generic *p);
        void sendPos();
        
    friend int physics_thread(Client *client);
    friend int packets_thread(Client *client);
    friend void renderWorld(Client *client);
};

#endif

