# C++ terminal plotting library

***WARNING: This library is a work in progress!***

Header-only.

Compile examples with C++14 support; link with `-lm`. The library tries to
detect terminal capabilities automatically; a xterm-compatible terminal
is recommended.

A makefile is provided in the `examples` directory. A CMake project and
a Code::Blocks project are also included in the `projects` directory
for IDE support.

Please note that non-POSIX platforms (e.g. Windows) are currently not
supported and compilation for them will fail.

## Build script

`build.py` is a python script which can pack all headers into a
single-header library. The following command will generate a
header file (`plot.hpp`) ready for inclusion in other projects.

```sh
./build.py braille.hpp real_canvas.hpp layout.hpp > plot.hpp
```

## Demo

[https://youtu.be/7WG6xP5MIe4](https://youtu.be/7WG6xP5MIe4)

[![Video: plot at work](https://img.youtube.com/vi/7WG6xP5MIe4/0.jpg)](https://www.youtube.com/watch?v=7WG6xP5MIe4)

## License

The MIT License

Copyright &copy; 2016 Fabio Massaioli

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
