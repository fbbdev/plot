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

#include <color.hpp>

//http://prideout.net/archive/colors.php

namespace ColorPicker {

constexpr plot::Color aliceblue{0.941f, 0.973f, 1.000f};
constexpr plot::Color antiquewhite{0.980f, 0.922f, 0.843f};
constexpr plot::Color aqua{0.000f, 1.000f, 1.000f};
constexpr plot::Color aquamarine{0.498f, 1.000f, 0.831f};
constexpr plot::Color azure{0.941f, 1.000f, 1.000f};
constexpr plot::Color beige{0.961f, 0.961f, 0.863f};
constexpr plot::Color bisque{1.000f, 0.894f, 0.769f};
constexpr plot::Color black{0.000f, 0.000f, 0.000f};
constexpr plot::Color blanchedalmond{1.000f, 0.922f, 0.804f};
constexpr plot::Color blue{0.000f, 0.000f, 1.000f};
constexpr plot::Color blueviolet{0.541f, 0.169f, 0.886f};
constexpr plot::Color brown{0.647f, 0.165f, 0.165f};
constexpr plot::Color burlywood{0.871f, 0.722f, 0.529f};
constexpr plot::Color cadetblue{0.373f, 0.620f, 0.627f};
constexpr plot::Color chartreuse{0.498f, 1.000f, 0.000f};
constexpr plot::Color chocolate{0.824f, 0.412f, 0.118f};
constexpr plot::Color coral{1.000f, 0.498f, 0.314f};
constexpr plot::Color cornflowerblue{0.392f, 0.584f, 0.929f};
constexpr plot::Color cornsilk{1.000f, 0.973f, 0.863f};
constexpr plot::Color crimson{0.863f, 0.078f, 0.235f};
constexpr plot::Color cyan{0.000f, 1.000f, 1.000f};
constexpr plot::Color darkblue{0.000f, 0.000f, 0.545f};
constexpr plot::Color darkcyan{0.000f, 0.545f, 0.545f};
constexpr plot::Color darkgoldenrod{0.722f, 0.525f, 0.043f};
constexpr plot::Color darkgray{0.663f, 0.663f, 0.663f};
constexpr plot::Color darkgreen{0.000f, 0.392f, 0.000f};
constexpr plot::Color darkgrey{0.663f, 0.663f, 0.663f};
constexpr plot::Color darkkhaki{0.741f, 0.718f, 0.420f};
constexpr plot::Color darkmagenta{0.545f, 0.000f, 0.545f};
constexpr plot::Color darkolivegreen{0.333f, 0.420f, 0.184f};
constexpr plot::Color darkorange{1.000f, 0.549f, 0.000f};
constexpr plot::Color darkorchid{0.600f, 0.196f, 0.800f};
constexpr plot::Color darkred{0.545f, 0.000f, 0.000f};
constexpr plot::Color darksalmon{0.914f, 0.588f, 0.478f};
constexpr plot::Color darkseagreen{0.561f, 0.737f, 0.561f};
constexpr plot::Color darkslateblue{0.282f, 0.239f, 0.545f};
constexpr plot::Color darkslategray{0.184f, 0.310f, 0.310f};
constexpr plot::Color darkslategrey{0.184f, 0.310f, 0.310f};
constexpr plot::Color darkturquoise{0.000f, 0.808f, 0.820f};
constexpr plot::Color darkviolet{0.580f, 0.000f, 0.827f};
constexpr plot::Color deeppink{1.000f, 0.078f, 0.576f};
constexpr plot::Color deepskyblue{0.000f, 0.749f, 1.000f};
constexpr plot::Color dimgray{0.412f, 0.412f, 0.412f};
constexpr plot::Color dimgrey{0.412f, 0.412f, 0.412f};
constexpr plot::Color dodgerblue{0.118f, 0.565f, 1.000f};
constexpr plot::Color firebrick{0.698f, 0.133f, 0.133f};
constexpr plot::Color floralwhite{1.000f, 0.980f, 0.941f};
constexpr plot::Color forestgreen{0.133f, 0.545f, 0.133f};
constexpr plot::Color fuchsia{1.000f, 0.000f, 1.000f};
constexpr plot::Color gainsboro{0.863f, 0.863f, 0.863f};
constexpr plot::Color ghostwhite{0.973f, 0.973f, 1.000f};
constexpr plot::Color gold{1.000f, 0.843f, 0.000f};
constexpr plot::Color goldenrod{0.855f, 0.647f, 0.125f};
constexpr plot::Color gray{0.502f, 0.502f, 0.502f};
constexpr plot::Color green{0.000f, 0.502f, 0.000f};
constexpr plot::Color greenyellow{0.678f, 1.000f, 0.184f};
constexpr plot::Color grey{0.502f, 0.502f, 0.502f};
constexpr plot::Color honeydew{0.941f, 1.000f, 0.941f};
constexpr plot::Color hotpink{1.000f, 0.412f, 0.706f};
constexpr plot::Color indianred{0.804f, 0.361f, 0.361f};
constexpr plot::Color indigo{0.294f, 0.000f, 0.510f};
constexpr plot::Color ivory{1.000f, 1.000f, 0.941f};
constexpr plot::Color khaki{0.941f, 0.902f, 0.549f};
constexpr plot::Color lavender{0.902f, 0.902f, 0.980f};
constexpr plot::Color lavenderblush{1.000f, 0.941f, 0.961f};
constexpr plot::Color lawngreen{0.486f, 0.988f, 0.000f};
constexpr plot::Color lemonchiffon{1.000f, 0.980f, 0.804f};
constexpr plot::Color lightblue{0.678f, 0.847f, 0.902f};
constexpr plot::Color lightcoral{0.941f, 0.502f, 0.502f};
constexpr plot::Color lightcyan{0.878f, 1.000f, 1.000f};
constexpr plot::Color lightgoldenrodyellow{0.980f, 0.980f, 0.824f};
constexpr plot::Color lightgray{0.827f, 0.827f, 0.827f};
constexpr plot::Color lightgreen{0.565f, 0.933f, 0.565f};
constexpr plot::Color lightgrey{0.827f, 0.827f, 0.827f};
constexpr plot::Color lightpink{1.000f, 0.714f, 0.757f};
constexpr plot::Color lightsalmon{1.000f, 0.627f, 0.478f};
constexpr plot::Color lightseagreen{0.125f, 0.698f, 0.667f};
constexpr plot::Color lightskyblue{0.529f, 0.808f, 0.980f};
constexpr plot::Color lightslategray{0.467f, 0.533f, 0.600f};
constexpr plot::Color lightslategrey{0.467f, 0.533f, 0.600f};
constexpr plot::Color lightsteelblue{0.690f, 0.769f, 0.871f};
constexpr plot::Color lightyellow{1.000f, 1.000f, 0.878f};
constexpr plot::Color lime{0.000f, 1.000f, 0.000f};
constexpr plot::Color limegreen{0.196f, 0.804f, 0.196f};
constexpr plot::Color linen{0.980f, 0.941f, 0.902f};
constexpr plot::Color magenta{1.000f, 0.000f, 1.000f};
constexpr plot::Color maroon{0.502f, 0.000f, 0.000f};
constexpr plot::Color mediumaquamarine{0.400f, 0.804f, 0.667f};
constexpr plot::Color mediumblue{0.000f, 0.000f, 0.804f};
constexpr plot::Color mediumorchid{0.729f, 0.333f, 0.827f};
constexpr plot::Color mediumpurple{0.576f, 0.439f, 0.859f};
constexpr plot::Color mediumseagreen{0.235f, 0.702f, 0.443f};
constexpr plot::Color mediumslateblue{0.482f, 0.408f, 0.933f};
constexpr plot::Color mediumspringgreen{0.000f, 0.980f, 0.604f};
constexpr plot::Color mediumturquoise{0.282f, 0.820f, 0.800f};
constexpr plot::Color mediumvioletred{0.780f, 0.082f, 0.522f};
constexpr plot::Color midnightblue{0.098f, 0.098f, 0.439f};
constexpr plot::Color mintcream{0.961f, 1.000f, 0.980f};
constexpr plot::Color mistyrose{1.000f, 0.894f, 0.882f};
constexpr plot::Color moccasin{1.000f, 0.894f, 0.710f};
constexpr plot::Color navajowhite{1.000f, 0.871f, 0.678f};
constexpr plot::Color navy{0.000f, 0.000f, 0.502f};
constexpr plot::Color oldlace{0.992f, 0.961f, 0.902f};
constexpr plot::Color olive{0.502f, 0.502f, 0.000f};
constexpr plot::Color olivedrab{0.420f, 0.557f, 0.137f};
constexpr plot::Color orange{1.000f, 0.647f, 0.000f};
constexpr plot::Color orangered{1.000f, 0.271f, 0.000f};
constexpr plot::Color orchid{0.855f, 0.439f, 0.839f};
constexpr plot::Color palegoldenrod{0.933f, 0.910f, 0.667f};
constexpr plot::Color palegreen{0.596f, 0.984f, 0.596f};
constexpr plot::Color paleturquoise{0.686f, 0.933f, 0.933f};
constexpr plot::Color palevioletred{0.859f, 0.439f, 0.576f};
constexpr plot::Color papayawhip{1.000f, 0.937f, 0.835f};
constexpr plot::Color peachpuff{1.000f, 0.855f, 0.725f};
constexpr plot::Color peru{0.804f, 0.522f, 0.247f};
constexpr plot::Color pink{1.000f, 0.753f, 0.796f};
constexpr plot::Color plum{0.867f, 0.627f, 0.867f};
constexpr plot::Color powderblue{0.690f, 0.878f, 0.902f};
constexpr plot::Color purple{0.502f, 0.000f, 0.502f};
constexpr plot::Color red{1.000f, 0.000f, 0.000f};
constexpr plot::Color rosybrown{0.737f, 0.561f, 0.561f};
constexpr plot::Color royalblue{0.255f, 0.412f, 0.882f};
constexpr plot::Color saddlebrown{0.545f, 0.271f, 0.075f};
constexpr plot::Color salmon{0.980f, 0.502f, 0.447f};
constexpr plot::Color sandybrown{0.957f, 0.643f, 0.376f};
constexpr plot::Color seagreen{0.180f, 0.545f, 0.341f};
constexpr plot::Color seashell{1.000f, 0.961f, 0.933f};
constexpr plot::Color sienna{0.627f, 0.322f, 0.176f};
constexpr plot::Color silver{0.753f, 0.753f, 0.753f};
constexpr plot::Color skyblue{0.529f, 0.808f, 0.922f};
constexpr plot::Color slateblue{0.416f, 0.353f, 0.804f};
constexpr plot::Color slategray{0.439f, 0.502f, 0.565f};
constexpr plot::Color slategrey{0.439f, 0.502f, 0.565f};
constexpr plot::Color snow{1.000f, 0.980f, 0.980f};
constexpr plot::Color springgreen{0.000f, 1.000f, 0.498f};
constexpr plot::Color steelblue{0.275f, 0.510f, 0.706f};
constexpr plot::Color tan{0.824f, 0.706f, 0.549f};
constexpr plot::Color teal{0.000f, 0.502f, 0.502f};
constexpr plot::Color thistle{0.847f, 0.749f, 0.847f};
constexpr plot::Color tomato{1.000f, 0.388f, 0.278f};
constexpr plot::Color turquoise{0.251f, 0.878f, 0.816f};
constexpr plot::Color violet{0.933f, 0.510f, 0.933f};
constexpr plot::Color wheat{0.961f, 0.871f, 0.702f};
constexpr plot::Color white{1.000f, 1.000f, 1.000f};
constexpr plot::Color whitesmoke{0.961f, 0.961f, 0.961f};
constexpr plot::Color yellow{1.000f, 1.000f, 0.000f};
constexpr plot::Color yellowgreen{0.604f, 0.804f, 0.196f};
}
