CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core

all: filter memecluster
opt: CXXFLAGS += -O3
opt: LDFLAGS += -O3
opt: filter memecluster
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: filter memecluster

filter: filter.cpp Snap.o dataloader.o doc.o quote.o lsh.o quotegraph.o clustering.o
	g++ -o filter filter.cpp Snap.o dataloader.o doc.o quote.o lsh.o quotegraph.o clustering.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 
	
memecluster: memecluster.cpp Snap.o dataloader.o doc.o quote.o lsh.o quotegraph.o clustering.o
	g++ -o memecluster memecluster.cpp Snap.o dataloader.o doc.o quote.o lsh.o quotegraph.o clustering.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
dataloader.o:
	g++ -c $(CXXFLAGS) dataloader.cpp -I$(GLIB) -I$(SNAP)
	
doc.o:
	g++ -c $(CXXFLAGS) doc.cpp -I$(GLIB) -I$(SNAP)
	
quote.o:
	g++ -c $(CXXFLAGS) quote.cpp -I$(GLIB) -I$(SNAP)

lsh.o:
	g++ -c $(CXXFLAGS) lsh.cpp -I$(GLIB) -I$(SNAP)

quotegraph.o:
	g++ -c $(CXXFLAGS) quotegraph.cpp -I$(GLIB) -I$(SNAP)
	
clustering.o:
	g++ -c $(CXXFLAGS) clustering.cpp -I$(GLIB) -I$(SNAP)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
