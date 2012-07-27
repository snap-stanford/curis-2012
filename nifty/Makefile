CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NLIB = lib

# all: memefilter memecluster memepostcluster memeincrement memeincrementalcluster
# opt: CXXFLAGS += -O3
# opt: LDFLAGS += -O3
# opt: memefilter memecluster memepostcluster memeincrement memeincrementalcluster
#profile: CXXFLAGS += -pg
#profile: LDFLAGS += -pg
#profile: memefilter memecluster memepostcluster memeincrement memeincrementalcluster

all: memefilter memecluster memepostcluster memeincrementalcluster
opt: CXXFLAGS += -O3
opt: LDFLAGS += -O3
opt: memefilter memecluster memepostcluster memeincrementalcluster
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: memefilter memecluster memepostcluster memeincrementalcluster

memefilter: memefilter.cpp Snap.o niftylib.o
	g++ -o memefilter memefilter.cpp Snap.o niftylib.o -fopenmp -I$(GLIB) -I$(SNAP) $(LDFLAGS) 
	
memecluster: memecluster.cpp Snap.o niftylib.o
	g++ -o memecluster memecluster.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

memeincrementalcluster: memeincrementalcluster.cpp Snap.o niftylib.o
	g++ -o memeincrementalcluster memeincrementalcluster.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

memepostcluster: memepostcluster.cpp Snap.o niftylib.o
	g++ -o memepostcluster memepostcluster.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)
	
#memeincrement: memeincrement.cpp Snap.o niftylib.o
#	g++ -o memeincrement memeincrement.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
niftylib.o:
	g++ -c $(CXXFLAGS) $(NLIB)/niftylib.cpp -I$(GLIB) -I$(SNAP)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
