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
    plot.hpp
    braille.hpp
    color.hpp
    layout.hpp
    point.hpp
    real_canvas.hpp
    rect.hpp
    string_view.hpp
    terminal.hpp
    utils.hpp)

string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/\\1" HEADER_FILES "${HEADER_FILES}")

add_library(plot INTERFACE)
target_sources(plot INTERFACE ${HEADER_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(examples)
