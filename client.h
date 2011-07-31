#include "SDL/SDL_thread.h"
#include "SDL/SDL_net.h"

class Client {
    public:
        static void init();
        static void quit();
    
        Client();
        virtual ~Client();
        
        bool connect(char *host, int port = 25565);
        bool login(char *username);
        
        bool running();
        
    private:
        bool connected;
        TCPsocket socket;
        bool doPhysics,doPackets;
        SDL_Thread *physics,*packets;
        
        void sendPos();
        
    friend int physics_thread(Client *client);
    friend int packets_thread(Client *client);
};
