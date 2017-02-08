/**
 * The MIT License
 *
 * Copyright (c) 2016 Fabio Massaioli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "../plot.hpp"

#include <cmath>
#include <iostream>
#include "opengl_colors.hpp"

using namespace plot;

//Create a function for returning the start and end stroke across a cell
//the Coord x is the current x location within the bounding box.
std::pair<Coord,Coord> sinPlotFunction(const Coord x)
{
    Coord base = 55 - std::lround(10*std::sin(2*3.141592f*((x - 12)/30.0f)));
    Coord end  = 55 - std::lround(10*std::sin(2*3.141592f*((x - 11)/30.0f)));
    return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
};

int main() {
    //Each Braille Canvas is made up of cells that are 2x4 points
    constexpr Coord canvasCellCols=70;
    constexpr Coord canvasCellRows=20;
    constexpr Size canvasCellSize{canvasCellCols,canvasCellRows};
    BrailleCanvas canvas(ColorPicker::floralwhite,canvasCellSize, TerminalInfo().detect());
    //The grid of the canvas is 70*2 x 20*4 or 140x80 points
    //so all future coordinates for where to draw objects are
    //relative to the 140x80 points with the origin (i.e. Point{0,0}) in the upper
    //left hand corner.

    //First draw a rectangle with a 'firebrick' outline and 'blueviolet; filling from point
    // location 11,11 to 40,40
    constexpr GenericPoint<Coord> upperLeft{11,11};
    constexpr GenericPoint<Coord> lowerRight{40,40};
    constexpr Rect FilledRectangleSize{ upperLeft, lowerRight };
    canvas.rect(ColorPicker::firebrick, ColorPicker::blueviolet, FilledRectangleSize);

    //next push lines in 'limegreen' overlayed onto the canvas
    //note that each method returns a reference to the object so
    //that commands can be easily chained together.
    canvas.push()
              .line(ColorPicker::limegreen, { 12, 17 }, { 17, 39 })
              .line(ColorPicker::limegreen, { 17, 39 }, { 39, 34 })
              .line(ColorPicker::limegreen, { 39, 34 }, { 34, 12 })
              .line(ColorPicker::limegreen, { 34, 12 }, { 12, 17 })
          .pop(TerminalOp::ClipDst);

    //Plot an ellipse in a bounding box from {0,0} to {30,30} offset by {45,11}
    constexpr Rect greyEllipseBoundingBox=Rect({ 30, 30 }) + Point(45, 11);
    canvas.ellipse(ColorPicker::slategrey, greyEllipseBoundingBox);
    //Plot an elipse with green line, filled with yellow centered at {60,26} with  semi-szis of {10,12}
    canvas.ellipse(ColorPicker::green, ColorPicker::yellow, { 60, 26 }, { 10, 15 });

    //Plot a function in 'mediumblue' color
    //Bounding box for where the stroke functions are rendered.
    constexpr Coord xStart=12;
    constexpr Coord xStop=71;
    constexpr Coord yStart=42;
    constexpr Coord yStop=67;
    constexpr Rect functionRectArea{ { xStart, yStart }, { xStop, yStop } };
    canvas.rect(ColorPicker::lightcyan,functionRectArea);
    canvas.push();

    //The function 'sinPlotFunction' will be evaluated at each value in [xStart, xStop)
    //and stroke in the color of 'mediumblue' will be rendered for those coordinates.
    canvas.stroke(ColorPicker::mediumblue, functionRectArea , sinPlotFunction);
    //Plot using a lambda function for cos
    canvas.stroke(ColorPicker::crimson, functionRectArea, [](Coord x) {
              constexpr Coord amplitude=10;
              Coord base = (yStop+yStart)/2 - std::lround(amplitude*std::cos(2*3.141592f*((x - xStart)/30.0f))),
                    end  = (yStop+yStart)/2 - std::lround(amplitude*std::cos(2*3.141592f*((x - xStart-1)/30.0f)));
              return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1);
          }, TerminalOp::ClipSrc);
     canvas.pop(TerminalOp::ClipDst);
     //Place a dot in each corner of the pixel grid
     canvas.dot(ColorPicker::orange,{0,0});
     canvas.dot(ColorPicker::purple,{0,80-1});
     canvas.dot(ColorPicker::gold,{140-1,0});
     canvas.dot(ColorPicker::indigo,{140-1,80-1});

    std::cout << canvas << std::endl;

    return 0;
}
