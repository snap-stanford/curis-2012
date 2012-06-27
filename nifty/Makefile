CXXFLAGS += -std=c++98 -g -Wall
LDFLAGS += -lrt


NAME = hello-world
SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core

all: $(NAME)
opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(NAME)

$(NAME): $(NAME).cpp Snap.o
	g++ -o $(NAME) $(NAME).cpp Snap.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

Snap.o: 
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)

clean:	
	rm -f *.o *.png *.plt *.tab $(NAME) $(NAME).exe