/**
 *  Copyright 2011 by Benjamin J. Land (a.k.a. BenLand100)
 *
 *  This file is part of the CppCraft.
 *
 *  CppCraft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CppCraft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CppCraft. If not, see <http://www.gnu.org/licenses/>.
 */

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
        
        void relLook(double dpitch, double dyaw); //adds the given offsets to the look direction (degrees)
        void setMotion(double forwards, double sideways); //moves at the given velocity
        void jump(); //jumps
        void startDigging(); //try to dig everything that is targeted
        void stopDigging(); //stop any digging activities
        void placeHeld(); //tries to place the held block on the target
        Block* getTarget(int &x, int &y, int &z, int &face); //returns the block we are looking at, or NULL if there is no target
        
        bool running(); //whether the connection is still good
        
        void lockUs(); //keep the physics_thread from clobbering things
        void unlockUs();
        
    private:
        World world;
        Player *us;
        bool onGround;
        bool connected;
        TCPsocket socket;
        bool doPhysics,doPackets;
        SDL_Thread *physics,*packets;
        SDL_mutex *usLock;
        
        double forwards,sideways; //our motion, set by setMotion, managed by physics_thread
        bool digging; int digStartTime; //set by startDigging/stopDigging, managed by physics_thread
        Block *target; int targetx,targety,targetz,targetface; //the block we are looking at, managed by physics_thread
        Block *beingdug; //the block we are digging, managed by physics_thread
        
        void packet(p_generic *p);
        void sendPos();
        void retarget();
        
    friend int physics_thread(Client *client);
    friend int packets_thread(Client *client);
    friend void renderWorld(Client *client);
    friend void renderHUD(Client *client);
};

#endif

