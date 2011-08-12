#include "entities.h"
#include <cmath>

Entity::Entity() : eid(0), x(0.0), y(0.0), z(0.0), height(0.0), vx(0.0), vy(0.0), vz(0.0), pitch(0.0), yaw(0.0) {
}

Entity::~Entity() {

}

Player::Player(char *name) : Entity(), apothem(0.40) {
    int len = strlen(name);
    this->name = new char[len+1];
    strcpy(this->name,name);
}

Player::~Player() {
    delete name;
}


void Player::boundingBox(int &sx,int &sy,int &sz,int &ex,int &ey,int &ez) {
    sx = floor(x-apothem);
    sy = floor(y);
    sz = floor(z-apothem);
    ex = floor(x+apothem);
    ey = floor(y+1.74);
    ez = floor(z+apothem);
}

Mob::Mob() : Entity() {

}

Mob::~Mob() {

}

