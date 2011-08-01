#include "entities.h"

Entity::Entity() {

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

