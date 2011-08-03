#!/bin/bash

g++ `sdl-config --cflags` -O2 *.cpp -pthread -lSDL -lSDL_net -lz -lGL -lGLU -o cppcraft
