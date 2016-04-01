USE_SDL := 1

# This will likely need to be changed to suit your installation.
ALE := $(HOME)/Programacio/Arcade-Learning-Environment

FLAGS := -I$(ALE)/src -I$(ALE)/src/controllers -I$(ALE)/src/os_dependent -I$(ALE)/src/environment -I$(ALE)/src/external -L$(ALE) -DTFG_DIR="\"$(HOME)/Dropbox/TFG\""
CXX := g++
LDFLAGS := -lale -lz -lSDL

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    FLAGS += -Wl,-rpath=$(ALE)
endif
ifeq ($(UNAME_S),Darwin)
    FLAGS += -framework Cocoa
endif

ifeq ($(strip $(USE_SDL)), 1)
  DEFINES += -D__USE_SDL -DSOUND_SUPPORT
  FLAGS += $(shell sdl-config --cflags)
  LDFLAGS += $(shell sdl-config --libs)
endif

default: all

all: hexq

hexq: hexq.cpp directed_graph.cpp directed_graph.hpp explained_assert.hpp hexq_level.cpp hexq_level.hpp
	$(CXX) -g --std=c++11 $(DEFINES) $(FLAGS) hexq.cpp directed_graph.cpp hexq_level.cpp $(LDFLAGS) -o hexq

clean:
	rm -rf hexq *.o *.graph *.dot *.png *.dSYM
.PHONY: clean

graphs:
	for f in *.dot; do dot -Tpng $$f > $$f.png; done
.PHONY: graphs
