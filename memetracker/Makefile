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

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core

all: MemeWorks niftytoqtbs

opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: MemeWorks niftytoqtbs

# COMPILE
memes.o: memes.cpp memes.h
	g++ -c $(CXXFLAGS) memes.cpp -I$(GLIB) -I$(SNAP) $(CXXFLAGS) 
	
niftytoqtbs: niftytoqtbs.cpp Snap.o memes.o spinn3r.o
	g++ $(LDFLAGS) -o niftytoqtbs niftytoqtbs.cpp memes.o spinn3r.o Snap.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 
	
MemeWorks: MemeWorks.cpp Snap.o memes.o spinn3r.o
	g++ $(LDFLAGS) -o MemeWorks MemeWorks.cpp memes.o spinn3r.o Snap.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

spinn3r.o: spinn3r.cpp spinn3r.h
	g++ -c $(CXXFLAGS) spinn3r.cpp -I$(GLIB) -I$(SNAP) $(CXXFLAGS) 
	
Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP) $(CXXFLAGS) 

clean:
	rm -f *.o  MemeWorks  MemeWorks.exe

