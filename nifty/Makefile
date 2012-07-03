CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt

NAME = filter
SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core

all: $(NAME)
opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(NAME)

$(NAME): $(NAME).cpp Snap.o dataloader.o doc.o quote.o lsh.o
	g++ -o $(NAME) $(NAME).cpp Snap.o dataloader.o doc.o quote.o lsh.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)
	
data_loader.o:
	g++ -c $(CXXFLAGS) data_loader.cpp -I$(GLIB) -I$(SNAP)
	
doc.o:
	g++ -c $(CXXFLAGS) doc.cpp -I$(GLIB) -I$(SNAP)
	
quote.o:
	g++ -c $(CXXFLAGS) quote.cpp -I$(GLIB) -I$(SNAP)

lsh.o:
	g++ -c $(CXXFLAGS) lsh.cpp -I$(GLIB) -I$(SNAP)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
