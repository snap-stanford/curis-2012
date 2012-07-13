CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NIFTY = ../nifty

all: analyzepeaks

opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(NAME)

debug: CXXFLAGS += -rdynamic -O0 -g -ggdb3 -pg
debug: LDFLAGS += -rdynamic -O0 -g -ggdb3 -pg
debug: analyzepeaks

analyzepeaks: analyzepeaks.cpp Snap.o niftylib.o
	g++ -o analyzepeaks analyzepeaks.cpp Snap.o niftylib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)
	
Snap.o:
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)

niftylib.o:
	g++ -c $(CXXFLAGS) $(NIFTY)/niftylib.cpp -I$(GLIB) -I$(SNAP)

clean:  
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
