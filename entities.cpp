#include "entities.h"

Entity::Entity() : eid(0), x(0.0), y(0.0), z(0.0), height(0.0), vx(0.0), vy(0.0), vz(0.0), pitch(0.0), yaw(0.0) {
}

Entity::~Entity() {

}

Player::Player(char *name) : Entity() {
    int len = strlen(name);
    this->name = new char[len+1];
    strcpy(this->name,name);
}

Player::~Player() {
    delete name;
}

Mob::Mob() : Entity() {

}

Mob::~Mob() {

}

