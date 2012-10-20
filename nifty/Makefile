CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NLIB = lib

all: memefilter memecluster memepostcluster memeincrement memeseed memecombine memelog
opt: CXXFLAGS += -O3
opt: LDFLAGS += -O3
opt: memefilter memecluster memepostcluster memeincrement memeseed memecombine memelog
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: memefilter memecluster memepostcluster memeincrement memeseed memecombine memelog
log: LOG += -g -rdynamic
log: memefilter memecluster memepostcluster memeincrement memeseed memecombine memelog

memefilter: memefilter.cpp Snap.o niftylib.o
	g++ -o memefilter memefilter.cpp Snap.o niftylib.o -fopenmp -I$(GLIB) -I$(SNAP) $(LDFLAGS) 
	
memepostcluster: memepostcluster.cpp Snap.o niftylib.o
	g++ -o memepostcluster memepostcluster.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

memeincrement: memeincrement.cpp Snap.o niftylib.o
	g++ -o memeincrement memeincrement.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)
	
memeseed: memeseed.cpp Snap.o niftylib.o
	g++ -o memeseed memeseed.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) $(LOG)
	
memecombine: memecombine.cpp Snap.o niftylib.o
	g++ -o memecombine memecombine.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)
	
memelog: memelog.cpp Snap.o niftylib.o
	g++ -o memelog memelog.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
niftylib.o:
	g++ -c $(CXXFLAGS) $(NLIB)/niftylib.cpp -I$(GLIB) -I$(SNAP) $(LOG)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
