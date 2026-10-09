# c-stickrig
A tiny data-driven skeleton rig in C and raylib. A stick figure stands on screen, built from a joint table and forward kinematics. Step one of an experiment in how far a hand-built C codebase can go.

use: 
gcc main.c -o rigtest -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./rigtest
