default: all

CXX=g++
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -Werror
LIBS = -lm

ifdef DEBUG
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -flto -Ofast -mavx2 -DNDEBUG
endif

headers := $(wildcard ../*.hpp)

all: build/terminal build/canvas build/frame build/animation build/boxes

build:
	mkdir -p build

build/terminal: terminal.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/terminal terminal.cpp $(LIBS)

build/canvas: canvas.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/canvas canvas.cpp $(LIBS)

build/frame: frame.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/frame frame.cpp $(LIBS)

build/animation: animation.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/animation animation.cpp $(LIBS)

build/boxes: boxes.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/boxes boxes.cpp $(LIBS)

clean:
	rm -rf build
