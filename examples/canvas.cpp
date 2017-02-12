/**
 * The MIT License
 *
 * Copyright (c) 2017 Fabio Massaioli
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

#include "plot.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace plot;

// Create a function returning the vertical start and end of the stroke
// at horizontal coordinate x. Pixels will be painted on vertical range [base,end)
std::pair<Coord,Coord> sinStrokeFunction(Coord x) {
    // To understand the apparently random numbers below (58, 10, 12 and 11),
    // see line 115
    Coord base = 58 - std::lround(10*std::sin(2*3.141592f*((x - 12)/30.0f)));
    Coord end  = 58 - std::lround(10*std::sin(2*3.141592f*((x - 11)/30.0f)));
    // When drawing a horizontal segment, base equals end and
    // stroke would have 0 width: so we make it 1px wide
    return std::make_pair(base, (base != end) ? end : base + 1);
}

int main() {
    // Each Braille Canvas is made up of cells that are 2x4 points
    // Points are switched on and off individually, but color is stored
    // per cell.
    constexpr Coord canvasCellCols = 70;
    constexpr Coord canvasCellRows = 20;
    constexpr Size canvasCellSize(canvasCellCols, canvasCellRows);

    BrailleCanvas canvas(canvasCellSize, TerminalInfo().detect());
    // The grid of the canvas is 70*2 x 20*4 or 140x80 points
    // so all future coordinates for where to draw objects are
    // relative to the 140x80 points with the origin (i.e. Point{0,0}) in the upper
    // left hand corner.

    // First draw a rectangle with a 'firebrick' outline and 'blueviolet' filling from point
    // location 11,11 to 40,40
    constexpr Point upperLeft(11, 11);
    constexpr Point lowerRight(40, 40);
    constexpr Rect filledRectangle(upperLeft, lowerRight);
    canvas.rect(palette::firebrick, palette::blueviolet, filledRectangle);

    // Draw lines in 'limegreen' overlayed onto the canvas
    // note that each method returns a reference to the object so
    // that commands can be easily chained together.

    // Push the current image to a stack and create a new clean image
    canvas.push()
          .line(palette::limegreen, { 12, 17 }, { 17, 39 })
          .line(palette::limegreen, { 17, 39 }, { 39, 34 })
          .line(palette::limegreen, { 39, 34 }, { 34, 12 })
          .line(palette::limegreen, { 34, 12 }, { 12, 17 });

    // Pop the previous image from the stack and composite the current
    // one onto it
    //
    // Most drawing commands can take an optional compositing operation
    // as their last argument. Three operations are available:
    //   - TerminalOp::Over = Paint source over destination, mix cell colors
    //   - TerminalOp::ClipDst = Erase destination cell where source is not empty
    //   - TerminalOp::ClipSrc = Ignore source cell where destination is not empty
    canvas.pop(TerminalOp::ClipDst);

    // Draw an ellipse in a bounding box from {0,0} to {30,30} offset by {45,11}
    constexpr Rect greyEllipseBoundingBox = Rect({ 30, 30 }) + Point(45, 11);
    canvas.ellipse(palette::slategrey, greyEllipseBoundingBox);
    // Draw an elipse with green outline, filled with yellow centered at {60,26} with  semi-axes of {10,12}
    canvas.ellipse(palette::green, palette::yellow, { 60, 26 }, { 10, 12 });

    // Bounding box for where the stroke functions are rendered.
    constexpr Coord xStart = 12;
    constexpr Coord xStop  = 71;
    constexpr Coord yStart = 46;
    constexpr Coord yStop  = 71;
    constexpr Rect strokeArea({ xStart, yStart }, { xStop, yStop });
    canvas.rect(palette::lightcyan, strokeArea);

    canvas.push();

    // Stroke a custom shaped line in 'royalblue' color
    //
    // The function 'sinStrokeFunction' will be evaluated at each value in [xStart, xStop]
    // and stroke in the color of 'royalblue' will be rendered for those coordinates.
    // Output will be clipped to range [yStart, yStop].
    canvas.stroke(palette::royalblue, strokeArea, sinStrokeFunction);

    constexpr float amplitude = 10;
    constexpr Coord vCenter = (yStop + yStart) / 2;

    // Fill cosine area using a custom lambda function returning true for
    // points inside the colored area.
    canvas.fill(palette::salmon, strokeArea, [](Point p) {
        Coord value = vCenter - std::lround(amplitude*std::cos(2*3.141592f*((p.x - xStart)/30.0f)));

        if (value < vCenter)
            return p.y <= vCenter && p.y >= value;
        else
            return p.y >= vCenter && p.y <= value;
     }, TerminalOp::ClipSrc);
     canvas.pop(TerminalOp::Over);

    // Place a dot in each corner of the pixel grid
    canvas.dot(palette::orange, { 0, 0 });
    canvas.dot(palette::purple, { 0, canvas.size().y - 1 });
    canvas.dot(palette::gold,   { canvas.size().x - 1, 0 });
    canvas.dot(palette::indigo, canvas.size() - Point(1, 1));

    // Draw a chain of lines from a sequence of points (initializer list)
    canvas.path(palette::deepskyblue, {
        { 98, 30 },
        { 80, 12 },
        { 100, 15 },
        { 82, 25 }
    });

    std::vector<Point> points{
        { 98, 50 },
        { 80, 68 },
        { 100, 65 },
        { 82, 55 }
    };

    // Draw a chain of lines from a sequence of points (iterators)
    canvas.path(palette::mediumseagreen, points.begin(), points.end());

    // Write the canvas to stdout
    std::cout << canvas << std::endl;

    return 0;
}
