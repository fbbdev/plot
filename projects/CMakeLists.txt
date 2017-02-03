# The MIT License
#
# Copyright (c) 2017 Fabio Massaioli
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

cmake_minimum_required(VERSION 3.1)
project(plot CXX)

set(CMAKE_CXX_STANDARD 14)

set(HEADER_FILES
        ../braille.hpp
        ../color.hpp
        ../layout.hpp
        ../plot.hpp
        ../point.hpp
        ../real_canvas.hpp
        ../rect.hpp
        ../string_view.hpp
        ../terminal.hpp
        ../utils.hpp)

include_directories(..)
add_executable(terminal ../examples/terminal.cpp ${HEADER_FILES})
add_executable(canvas ../examples/canvas.cpp ${HEADER_FILES})
add_executable(real_canvas ../examples/real_canvas.cpp ${HEADER_FILES})
add_executable(frame ../examples/frame.cpp ${HEADER_FILES})
add_executable(animation ../examples/animation.cpp ${HEADER_FILES})
add_executable(boxes ../examples/boxes.cpp ${HEADER_FILES})
