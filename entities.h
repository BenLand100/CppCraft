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
        
        char *name;
};

class Mob : public Entity {
    public:
        Mob();
        virtual ~Mob();
        
        metadata meta;
        int mobtype;
};

#endif
