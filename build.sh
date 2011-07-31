#!/bin/bash

g++ `sdl-config --cflags` -g *.cpp -pthread -lSDL -lSDL_net -o cppcraft
