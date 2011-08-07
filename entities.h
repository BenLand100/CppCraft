#ifndef _entities
#define _entities

#include "packets.h"

class Entity {
    public:
        Entity();
        virtual ~Entity();
        
        int eid;
        double x,y,z,height;
        double vx,vy,vz;
        double pitch,yaw;
};

class Player : public Entity {
    public:
        Player(char *name);
        virtual ~Player();
        
        void boundingBox(int &sx,int &sy,int &sz,int &ex,int &ey,int &ez);
        
        char *name;
        double apothem; //width from center to outside edge
};

class Mob : public Entity {
    public:
        Mob();
        virtual ~Mob();
        
        metadata meta;
        int mobtype;
};

#endif
