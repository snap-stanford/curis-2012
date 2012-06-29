# CXXFLAGS += -std=c++98 -g -Wall
# LDFLAGS += -lrt


NAME = memeclust
SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core

all: $(NAME)
opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(NAME)

$(NAME): $(NAME).cpp Snap.o spinn3r.o memes.o
	g++ -o $(NAME) $(NAME).cpp spinn3r.o Snap.o memes.o -I$(GLIB) -I$(SNAP) $(LDFLAGS)

spinn3r.o: $(SNAP)/spinn3r.cpp
	g++ -c $(CXXFLAGS) $(SNAP)/spinn3r.cpp -I$(GLIB) -I$(SNAP)	

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)

memes.o: memes.cpp
	g++ -c $(CXXFLAGS) memes.cpp -I$(GLIB) -I$(SNAP)
clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe
