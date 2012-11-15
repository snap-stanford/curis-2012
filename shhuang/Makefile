CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NLIB = ../nifty/lib

all: analyzedeletededges
opt: CXXFLAGS += -O3
opt: LDFLAGS += -O3
opt: analyzedeletededges
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: analyzedeletededges
log: LOG += -g -rdynamic
log: analyzedeletededges

analyzedeletededges: analyzedeletededges.cpp Snap.o niftylib.o
	g++ -o analyzedeletededges analyzedeletededges.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
niftylib.o:
	g++ -c $(CXXFLAGS) $(NLIB)/niftylib.cpp -I$(GLIB) -I$(SNAP) $(LOG)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
