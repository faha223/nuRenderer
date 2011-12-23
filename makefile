# This file makes shit!

#This is the default compiler
CXX = g++
# default compile flags
FLAGS = -Wall -c -g

# Main build target
all: x86 x64

x86: libRenderer32.o driver32

x64: libRenderer64.o driver64

# 32-bit renderer library
libRenderer32.o: renderer.h renderer.cpp libMatrix32.o libVector32.o makefile
	$(CXX) $(FLAGS) -m32 renderer.cpp -o libRenderer32.o

# 64-bit renderer library
libRenderer64.o: renderer.h renderer.cpp libMatrix64.o libVector64.o makefile
	$(CXX) $(FLAGS) -m64 renderer.cpp -o libRenderer64.o

# 64-bit driver
driver64: driver.cpp libRenderer64.o geometry.h
	$(CXX) -Wall -g -m64 libRenderer64.o libMatrix64.o libVector64.o driver.cpp -o driver64 -lSDL -lSDL_image -lSDL_ttf -lGL -lGLU

# 32-bit driver
driver32: driver.cpp libRenderer32.o geometry.h
	$(CXX) -Wall -g -m32 -L/usr/lib32 libRenderer32.o libMatrix32.o libVector32.o driver.cpp -o driver32 -lSDL -lSDL_image -lSDL_ttf -lGL -lGLU

# fake build target for clean-up
clean:
	rm -f libRenderer*
	rm -f driver32 driver64
