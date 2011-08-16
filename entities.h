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
