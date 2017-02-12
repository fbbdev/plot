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

using namespace plot;

// This example is identical to canvas.cpp, except it uses
// floating-point coordinates through the RealCanvas wrapper.

// Create a function returning the vertical start and end of the stroke
// for the pixel column delimited by horizontal coordinates x0 and x1.
// Pixels will be painted on vertical range [base,end).
std::pair<Coordf,Coordf> sinStrokeFunction(Coordf x0, Coordf x1) {
    // To understand the apparently random numbers below (0.422, 0.072, 0.086),
    // see line 135
    Coordf base = 0.422f - 0.072f*std::sin(2*3.141592f*((x0 - 0.086f)/0.211f));
    Coordf end  = 0.422f - 0.072f*std::sin(2*3.141592f*((x1 - 0.086f)/0.211f));

    // When stroke width (end - base) maps to less than 1px, it will be
    // rounded automatically to 1px
    return std::make_pair(base, end);
}

int main() {
    // Each Braille Canvas is made up of cells that are 2x4 points
    // Points are switched on and off individually, but color is stored
    // per cell.
    constexpr Coord canvasCellCols = 70;
    constexpr Coord canvasCellRows = 20;
    constexpr Size canvasCellSize(canvasCellCols, canvasCellRows);

    constexpr float aspectRatio = float(2*canvasCellCols) / float(4*canvasCellRows);

    // Real coordinate bounds:
    //   realCanvasBounds.p1.x and realCanvasBounds.p2.x will map respectively
    //   to the first and last pixel column of the underlying canvas.
    //
    //   Similarly, realCanvasBounds.p1.y and realCanvasBounds.p2.y will map
    //   to the first and last pixel row of the underlying canvas.
    //
    //   Start coordinates can be greater then end coordinates.
    constexpr Rectf realCanvasBounds({ 0.0f, 0.0f }, { 1.0f, 1.0f/aspectRatio });

    // RealCanvas wraps a canvas object (BrailleCanvas or one with a compatible API)
    // and maps a real coordinate space to integer pixel coordinates of the
    // underlying canvas.
    //
    // The constructor takes real coordinate bounds as optional first argument;
    // subsequent arguments are forwarded to the canvas object constructor.
    RealCanvas<BrailleCanvas> canvas({ { 0.0f, 0.0f }, { 1.0f, 0.57f } }, canvasCellSize, TerminalInfo().detect());

    // First draw a rectangle with a 'firebrick' outline and 'blueviolet' filling from point
    // location { 0.079, 0.079 } to { 0.288, 0.288 }
    constexpr Pointf upperLeft(0.079, 0.079);
    constexpr Pointf lowerRight(0.288, 0.288);
    constexpr Rectf filledRectangle(upperLeft, lowerRight);
    canvas.rect(palette::firebrick, palette::blueviolet, filledRectangle);

    // Draw lines in 'limegreen' overlayed onto the canvas
    // note that each method returns a reference to the object so
    // that commands can be easily chained together.

    // See canvas.cpp for a detailed explanation of the methods used below.
    canvas.push()
              .line(palette::limegreen, { 0.086f, 0.122f }, { 0.122f, 0.281f })
              .line(palette::limegreen, { 0.122f, 0.281f }, { 0.281f, 0.245f })
              .line(palette::limegreen, { 0.281f, 0.245f }, { 0.245f, 0.086f })
              .line(palette::limegreen, { 0.245f, 0.086f }, { 0.086f, 0.122f })
          .pop(TerminalOp::ClipDst);

    // Draw an ellipse in a bounding box from {0,0} to {0.214, 0.214} offset by {0.321, 0.079}
    constexpr Rectf greyEllipseBoundingBox = Rectf({ 0.214f, 0.214f }) + Pointf(0.321f, 0.079f);
    canvas.ellipse(palette::slategrey, greyEllipseBoundingBox);
    // Draw an elipse with green outline, filled with yellow centered at {0.432, 0.186} with  semi-axes of {0.072, 0.086}
    canvas.ellipse(palette::green, palette::yellow, { 0.432f, 0.186f }, { 0.072f, 0.086f });

    // Calculate bounding box for where the stroke functions are rendered.
    //
    // RealCanvas::unmap converts a Point or Rect from pixel coordinates to
    // real coordinates.
    // The corresponding method RealCanvas::map converts a Pointf or Rectf
    // from real coordinates to pixel coordinates.
    //
    // Methods RealCanvas::map_size and RealCanvas::unmap_size can be used
    // to convert absolute sizes to and from pixel coordinates.
    Rectf strokeArea = canvas.unmap(Rect({ 12, 46 }, { 71, 71 }));
    Coordf xStart = strokeArea.p1.x;
    Coordf yStart = strokeArea.p1.y;
    Coordf yStop  = strokeArea.p2.y;
    canvas.rect(palette::lightcyan, strokeArea);

    canvas.push();

    // Stroke a custom shaped line in 'royalblue' color
    //
    // The function 'sinStrokeFunction' will be evaluated for each pixel in the
    // range mapped to [xStart, xStop] and stroke in the color of 'royalblue'
    // will be rendered for those coordinates.
    // Output will be clipped to the pixel range mapped to [yStart, yStop].
    canvas.stroke(palette::royalblue, strokeArea, sinStrokeFunction);

    // Get width and height of one pixel in real space
    Sizef pixel = canvas.unmap_size({ 1, 1 });
    float amplitude = 10*pixel.y;
    Coordf vCenter = (yStop + yStart) / 2.0f;

    // Fill cosine area using a custom lambda function returning true for
    // points inside the colored area.
    canvas.fill(palette::salmon, strokeArea, [amplitude, xStart, vCenter](Pointf p) {
        Coordf value = vCenter - amplitude*std::cos(2*3.141592f*((p.x - xStart)/0.211f));

        if (value < vCenter)
            return p.y <= vCenter && p.y >= value;
        else
            return p.y >= vCenter && p.y <= value;
    }, TerminalOp::ClipSrc);
    canvas.pop(TerminalOp::Over);

    // Place a dot in each corner of the pixel grid
    canvas.dot(palette::orange, { 0.0f, 0.0f });
    canvas.dot(palette::purple, { 0, realCanvasBounds.p2.y });
    canvas.dot(palette::gold,   { realCanvasBounds.p2.x, 0 });
    canvas.dot(palette::indigo, realCanvasBounds.p2);

    // Draw a chain of lines from a sequence of points (initializer list)
    canvas.path(palette::deepskyblue, {
        { 98*pixel.x, 30*pixel.y },
        { 80*pixel.x, 12*pixel.y },
        { 100*pixel.x, 15*pixel.y },
        { 82*pixel.x, 25*pixel.y }
    });

    std::vector<Pointf> points{
        { 98*pixel.x, 50*pixel.y },
        { 80*pixel.x, 68*pixel.y },
        { 100*pixel.x, 65*pixel.y },
        { 82*pixel.x, 55*pixel.y }
    };

    // Draw a chain of lines from a sequence of points (iterators)
    canvas.path(palette::mediumseagreen, points.begin(), points.end());

    // Write the canvas to stdout
    std::cout << canvas << std::endl;

    return 0;
}
