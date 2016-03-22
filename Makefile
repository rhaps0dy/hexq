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

all: memory_record hexq

hexq: hexq.cpp DirectedGraph.cpp
	$(CXX) -g --std=c++11 $(DEFINES) $(FLAGS) hexq.cpp DirectedGraph.cpp $(LDFLAGS) -o hexq

memory_record: memory_record.cpp
	$(CXX) --std=c++11 $(DEFINES) $(FLAGS) memory_record.cpp $(LDFLAGS) -o memory_record

clean:
	rm -rf hexq memory_record *.o
