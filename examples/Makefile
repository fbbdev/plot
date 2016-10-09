default: all

CXX=g++
CXXFLAGS += -std=c++14
LIBS = -lm

ifdef DEBUG
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -flto -Ofast -mavx2 -DNDEBUG
endif

headers := $(wildcard ../*.hpp)

all: build/canvas build/border build/animation build/multi

build:
	mkdir -p build

build/canvas: canvas.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/canvas canvas.cpp $(LIBS)

build/border: border.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/border border.cpp $(LIBS)

build/animation: animation.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/animation animation.cpp $(LIBS)

build/multi: multi.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/multi multi.cpp $(LIBS)

clean:
	rm -rf build
