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

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(SEND_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_CXX_STANDARD 14)

set(HEADER_FILES
    braille.hpp
    color.hpp
    colors.hpp
    layout.hpp
    plot.hpp
    point.hpp
    real_canvas.hpp
    rect.hpp
    string_view.hpp
    terminal.hpp
    utils.hpp)

string(REGEX REPLACE "([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/\\1" HEADER_FILES "${HEADER_FILES}")

add_library(plot INTERFACE)
target_sources(plot INTERFACE ${HEADER_FILES})

option(BUILD_SINGLE_HEADER "Pack all headers into a single header library" OFF)

if(BUILD_SINGLE_HEADER)
    find_package(PythonInterp 3 REQUIRED)

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/plot.hpp"
        COMMAND "${PYTHON_EXECUTABLE}" "pack.py" "plot.hpp" > "${CMAKE_CURRENT_BINARY_DIR}/plot.hpp"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${HEADER_FILES}
        COMMENT "Packing header files")

    add_custom_target(
        single-header ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/plot.hpp")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(examples)
