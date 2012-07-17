CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NLIB = lib

all: filter memecluster
opt: CXXFLAGS += -O3
opt: LDFLAGS += -O3
opt: filter memecluster
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: filter memecluster

filter: filter.cpp Snap.o niftylib.o
	g++ -o filter filter.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 
	
memecluster: memecluster.cpp Snap.o niftylib.o
	g++ -o memecluster memecluster.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
niftylib.o:
	g++ -c $(CXXFLAGS) $(NLIB)/niftylib.cpp -I$(GLIB) -I$(SNAP)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
