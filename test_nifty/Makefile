# CXXFLAGS += -std=c++98 -g -Wall
# LDFLAGS += -lrt

NAME = doc_test
SNAP = ../../snap/snap-core
GLIB = ../../snap/glib-core
NIFTY = ../nifty

all: $(NAME)
opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(NAME)

$(NAME): $(NAME).cpp Snap.o Nifty_lib.o
	g++ -o $(NAME) $(NAME).cpp Snap.o Nifty_lib.o -I$(GLIB) -I$(SNAP) $(LDFLAGS) 

Snap.o:
	g++ -c $(CXXFLAGS) $(SNAP)/Snap.cpp -I$(GLIB) -I$(SNAP)

Nifty_lib.o:
	g++ -c $(CXXFLAGS) $(NIFTY)/Nifty_lib.cpp -I$(GLIB) -I$(SNAP)
        
