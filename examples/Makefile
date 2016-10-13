# The MIT License
#
# Copyright (c) 2016 Fabio Massaioli
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

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

all: build/terminal build/canvas build/real_canvas build/frame build/animation build/boxes

build:
	mkdir -p build

build/terminal: terminal.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/terminal terminal.cpp $(LIBS)

build/canvas: canvas.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/canvas canvas.cpp $(LIBS)

build/real_canvas: real_canvas.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/real_canvas real_canvas.cpp $(LIBS)

build/frame: frame.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/frame frame.cpp $(LIBS)

build/animation: animation.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/animation animation.cpp $(LIBS)

build/boxes: boxes.cpp $(headers) | build
	$(CXX) $(CXXFLAGS) -o build/boxes boxes.cpp $(LIBS)

clean:
	rm -rf build
