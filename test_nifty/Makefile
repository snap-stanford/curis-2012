CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NIFTY = ../nifty/lib

all: clusteringtest doctest lshtest quotetest clustertest

opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(NAME)

debug: CXXFLAGS += -rdynamic -O0 -g -ggdb3 -pg
debug: LDFLAGS += -rdynamic -O0 -g -ggdb3 -pg
debug: doctest lshtest quotetest clustertest

clusteringtest: clusteringtest.cpp Snap.o niftylib.o
	g++ -o clusteringtest clusteringtest.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

doctest: doctest.cpp Snap.o niftylib.o
	g++ -o doctest doctest.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

lshtest: lshtest.cpp Snap.o niftylib.o
	g++ -o lshtest lshtest.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

quotetest: quotetest.cpp Snap.o niftylib.o
	g++ -o quotetest quotetest.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

clustertest: clustertest.cpp Snap.o niftylib.o
	g++ -o clustertest clustertest.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o:
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)

niftylib.o:
	g++ -c $(CXXFLAGS) $(NIFTY)/niftylib.cpp -I$(GLIB) -I$(SNAP)

clean:  
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
