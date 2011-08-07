#!/bin/bash

g++ `sdl-config --cflags` -O3 *.cpp -pthread -lSDL -lSDL_net -lz -lGL -lGLU -o cppcraft
