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

add_executable(terminal terminal.cpp)
add_executable(canvas canvas.cpp)
add_executable(real_canvas real_canvas.cpp)
add_executable(frame frame.cpp)
add_executable(animation animation.cpp)
add_executable(boxes boxes.cpp)

set(LIBS plot)

if(UNIX)
    list(APPEND LIBS m)
endif()

target_link_libraries(terminal ${LIBS})
target_link_libraries(canvas ${LIBS})
target_link_libraries(real_canvas ${LIBS})
target_link_libraries(frame ${LIBS})
target_link_libraries(animation ${LIBS})
target_link_libraries(boxes ${LIBS})

add_executable(make_xterm_table make_xterm_table.cpp)
target_link_libraries(make_xterm_table ${LIBS})