CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NLIB = lib

all: memefilter memeincrement memeseed memelog memedumpquote
opt: CXXFLAGS += -O3
opt: LDFLAGS += -O3
opt: memefilter memeincrement memeseed memelog memedumpquote
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: memefilter memeincrement memeseed memelog memedumpquote
log: LOG += -g -rdynamic
log: memefilter memeincrement memeseed memelog memedumpquote

memedumpquote: memedumpquote.cpp Snap.o niftylib.o
	g++ -o memedumpquote memedumpquote.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

memefilter: memefilter.cpp Snap.o niftylib.o
	g++ -o memefilter memefilter.cpp Snap.o niftylib.o -fopenmp -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

memeincrement: memeincrement.cpp Snap.o niftylib.o
	g++ -o memeincrement memeincrement.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)
	
memeseed: memeseed.cpp Snap.o niftylib.o
	g++ -o memeseed memeseed.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) $(LOG)
	
memelog: memelog.cpp Snap.o niftylib.o
	g++ -o memelog memelog.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
niftylib.o:
	g++ -c $(CXXFLAGS) $(NLIB)/niftylib.cpp -I$(GLIB) -I$(SNAP) $(LOG)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
