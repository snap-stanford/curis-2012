#
# MemeTracker by Jure Leskovec
#
# Makefile for testing with non-Microsoft compilers
#

## Linux  (uncomment the 2 lines below for compilation on Linux)
CXXFLAGS += -std=c++98 -Wall
LDFLAGS += -lrt

## CygWin (uncomment the 2 lines below for compilation on CygWin)
#CXXFLAGS += -Wall
#LDFLAGS += 

GG = ../../../SnapLib

all: MemeWorks

opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: MemeWorks

# COMPILE
MemeWorks: MemeWorks.cpp Snap.o memes.o spinn3r.o
	g++ $(LDFLAGS) -o MemeWorks MemeWorks.cpp memes.o spinn3r.o Snap.o -I../../../GLib -I$(GG)

memes.o: $(GG)/memes.cpp $(GG)/memes.h
	g++ -c $(CXXFLAGS) $(GG)/memes.cpp -I../../../GLib -I$(GG)

spinn3r.o: $(GG)/spinn3r.cpp $(GG)/spinn3r.h
	g++ -c $(CXXFLAGS) $(GG)/spinn3r.cpp -I../../../GLib -I$(GG)
	
Snap.o: 
	g++ -c $(CXXFLAGS) $(GG)/Snap.cpp -I../../../GLib -I$(GG)

clean:
	rm -f *.o  MemeWorks  MemeWorks.exe

