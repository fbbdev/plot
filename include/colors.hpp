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

#include "color.hpp"

// http://prideout.net/archive/colors.php

namespace plot
{

namespace palette
{
    constexpr Color aliceblue{0.941f, 0.973f, 1.000f};
    constexpr Color antiquewhite{0.980f, 0.922f, 0.843f};
    constexpr Color aqua{0.000f, 1.000f, 1.000f};
    constexpr Color aquamarine{0.498f, 1.000f, 0.831f};
    constexpr Color azure{0.941f, 1.000f, 1.000f};
    constexpr Color beige{0.961f, 0.961f, 0.863f};
    constexpr Color bisque{1.000f, 0.894f, 0.769f};
    constexpr Color black{0.000f, 0.000f, 0.000f};
    constexpr Color blanchedalmond{1.000f, 0.922f, 0.804f};
    constexpr Color blue{0.000f, 0.000f, 1.000f};
    constexpr Color blueviolet{0.541f, 0.169f, 0.886f};
    constexpr Color brown{0.647f, 0.165f, 0.165f};
    constexpr Color burlywood{0.871f, 0.722f, 0.529f};
    constexpr Color cadetblue{0.373f, 0.620f, 0.627f};
    constexpr Color chartreuse{0.498f, 1.000f, 0.000f};
    constexpr Color chocolate{0.824f, 0.412f, 0.118f};
    constexpr Color coral{1.000f, 0.498f, 0.314f};
    constexpr Color cornflowerblue{0.392f, 0.584f, 0.929f};
    constexpr Color cornsilk{1.000f, 0.973f, 0.863f};
    constexpr Color crimson{0.863f, 0.078f, 0.235f};
    constexpr Color cyan{0.000f, 1.000f, 1.000f};
    constexpr Color darkblue{0.000f, 0.000f, 0.545f};
    constexpr Color darkcyan{0.000f, 0.545f, 0.545f};
    constexpr Color darkgoldenrod{0.722f, 0.525f, 0.043f};
    constexpr Color darkgray{0.663f, 0.663f, 0.663f};
    constexpr Color darkgreen{0.000f, 0.392f, 0.000f};
    constexpr Color darkgrey{0.663f, 0.663f, 0.663f};
    constexpr Color darkkhaki{0.741f, 0.718f, 0.420f};
    constexpr Color darkmagenta{0.545f, 0.000f, 0.545f};
    constexpr Color darkolivegreen{0.333f, 0.420f, 0.184f};
    constexpr Color darkorange{1.000f, 0.549f, 0.000f};
    constexpr Color darkorchid{0.600f, 0.196f, 0.800f};
    constexpr Color darkred{0.545f, 0.000f, 0.000f};
    constexpr Color darksalmon{0.914f, 0.588f, 0.478f};
    constexpr Color darkseagreen{0.561f, 0.737f, 0.561f};
    constexpr Color darkslateblue{0.282f, 0.239f, 0.545f};
    constexpr Color darkslategray{0.184f, 0.310f, 0.310f};
    constexpr Color darkslategrey{0.184f, 0.310f, 0.310f};
    constexpr Color darkturquoise{0.000f, 0.808f, 0.820f};
    constexpr Color darkviolet{0.580f, 0.000f, 0.827f};
    constexpr Color deeppink{1.000f, 0.078f, 0.576f};
    constexpr Color deepskyblue{0.000f, 0.749f, 1.000f};
    constexpr Color dimgray{0.412f, 0.412f, 0.412f};
    constexpr Color dimgrey{0.412f, 0.412f, 0.412f};
    constexpr Color dodgerblue{0.118f, 0.565f, 1.000f};
    constexpr Color firebrick{0.698f, 0.133f, 0.133f};
    constexpr Color floralwhite{1.000f, 0.980f, 0.941f};
    constexpr Color forestgreen{0.133f, 0.545f, 0.133f};
    constexpr Color fuchsia{1.000f, 0.000f, 1.000f};
    constexpr Color gainsboro{0.863f, 0.863f, 0.863f};
    constexpr Color ghostwhite{0.973f, 0.973f, 1.000f};
    constexpr Color gold{1.000f, 0.843f, 0.000f};
    constexpr Color goldenrod{0.855f, 0.647f, 0.125f};
    constexpr Color gray{0.502f, 0.502f, 0.502f};
    constexpr Color green{0.000f, 0.502f, 0.000f};
    constexpr Color greenyellow{0.678f, 1.000f, 0.184f};
    constexpr Color grey{0.502f, 0.502f, 0.502f};
    constexpr Color honeydew{0.941f, 1.000f, 0.941f};
    constexpr Color hotpink{1.000f, 0.412f, 0.706f};
    constexpr Color indianred{0.804f, 0.361f, 0.361f};
    constexpr Color indigo{0.294f, 0.000f, 0.510f};
    constexpr Color ivory{1.000f, 1.000f, 0.941f};
    constexpr Color khaki{0.941f, 0.902f, 0.549f};
    constexpr Color lavender{0.902f, 0.902f, 0.980f};
    constexpr Color lavenderblush{1.000f, 0.941f, 0.961f};
    constexpr Color lawngreen{0.486f, 0.988f, 0.000f};
    constexpr Color lemonchiffon{1.000f, 0.980f, 0.804f};
    constexpr Color lightblue{0.678f, 0.847f, 0.902f};
    constexpr Color lightcoral{0.941f, 0.502f, 0.502f};
    constexpr Color lightcyan{0.878f, 1.000f, 1.000f};
    constexpr Color lightgoldenrodyellow{0.980f, 0.980f, 0.824f};
    constexpr Color lightgray{0.827f, 0.827f, 0.827f};
    constexpr Color lightgreen{0.565f, 0.933f, 0.565f};
    constexpr Color lightgrey{0.827f, 0.827f, 0.827f};
    constexpr Color lightpink{1.000f, 0.714f, 0.757f};
    constexpr Color lightsalmon{1.000f, 0.627f, 0.478f};
    constexpr Color lightseagreen{0.125f, 0.698f, 0.667f};
    constexpr Color lightskyblue{0.529f, 0.808f, 0.980f};
    constexpr Color lightslategray{0.467f, 0.533f, 0.600f};
    constexpr Color lightslategrey{0.467f, 0.533f, 0.600f};
    constexpr Color lightsteelblue{0.690f, 0.769f, 0.871f};
    constexpr Color lightyellow{1.000f, 1.000f, 0.878f};
    constexpr Color lime{0.000f, 1.000f, 0.000f};
    constexpr Color limegreen{0.196f, 0.804f, 0.196f};
    constexpr Color linen{0.980f, 0.941f, 0.902f};
    constexpr Color magenta{1.000f, 0.000f, 1.000f};
    constexpr Color maroon{0.502f, 0.000f, 0.000f};
    constexpr Color mediumaquamarine{0.400f, 0.804f, 0.667f};
    constexpr Color mediumblue{0.000f, 0.000f, 0.804f};
    constexpr Color mediumorchid{0.729f, 0.333f, 0.827f};
    constexpr Color mediumpurple{0.576f, 0.439f, 0.859f};
    constexpr Color mediumseagreen{0.235f, 0.702f, 0.443f};
    constexpr Color mediumslateblue{0.482f, 0.408f, 0.933f};
    constexpr Color mediumspringgreen{0.000f, 0.980f, 0.604f};
    constexpr Color mediumturquoise{0.282f, 0.820f, 0.800f};
    constexpr Color mediumvioletred{0.780f, 0.082f, 0.522f};
    constexpr Color midnightblue{0.098f, 0.098f, 0.439f};
    constexpr Color mintcream{0.961f, 1.000f, 0.980f};
    constexpr Color mistyrose{1.000f, 0.894f, 0.882f};
    constexpr Color moccasin{1.000f, 0.894f, 0.710f};
    constexpr Color navajowhite{1.000f, 0.871f, 0.678f};
    constexpr Color navy{0.000f, 0.000f, 0.502f};
    constexpr Color oldlace{0.992f, 0.961f, 0.902f};
    constexpr Color olive{0.502f, 0.502f, 0.000f};
    constexpr Color olivedrab{0.420f, 0.557f, 0.137f};
    constexpr Color orange{1.000f, 0.647f, 0.000f};
    constexpr Color orangered{1.000f, 0.271f, 0.000f};
    constexpr Color orchid{0.855f, 0.439f, 0.839f};
    constexpr Color palegoldenrod{0.933f, 0.910f, 0.667f};
    constexpr Color palegreen{0.596f, 0.984f, 0.596f};
    constexpr Color paleturquoise{0.686f, 0.933f, 0.933f};
    constexpr Color palevioletred{0.859f, 0.439f, 0.576f};
    constexpr Color papayawhip{1.000f, 0.937f, 0.835f};
    constexpr Color peachpuff{1.000f, 0.855f, 0.725f};
    constexpr Color peru{0.804f, 0.522f, 0.247f};
    constexpr Color pink{1.000f, 0.753f, 0.796f};
    constexpr Color plum{0.867f, 0.627f, 0.867f};
    constexpr Color powderblue{0.690f, 0.878f, 0.902f};
    constexpr Color purple{0.502f, 0.000f, 0.502f};
    constexpr Color red{1.000f, 0.000f, 0.000f};
    constexpr Color rosybrown{0.737f, 0.561f, 0.561f};
    constexpr Color royalblue{0.255f, 0.412f, 0.882f};
    constexpr Color saddlebrown{0.545f, 0.271f, 0.075f};
    constexpr Color salmon{0.980f, 0.502f, 0.447f};
    constexpr Color sandybrown{0.957f, 0.643f, 0.376f};
    constexpr Color seagreen{0.180f, 0.545f, 0.341f};
    constexpr Color seashell{1.000f, 0.961f, 0.933f};
    constexpr Color sienna{0.627f, 0.322f, 0.176f};
    constexpr Color silver{0.753f, 0.753f, 0.753f};
    constexpr Color skyblue{0.529f, 0.808f, 0.922f};
    constexpr Color slateblue{0.416f, 0.353f, 0.804f};
    constexpr Color slategray{0.439f, 0.502f, 0.565f};
    constexpr Color slategrey{0.439f, 0.502f, 0.565f};
    constexpr Color snow{1.000f, 0.980f, 0.980f};
    constexpr Color springgreen{0.000f, 1.000f, 0.498f};
    constexpr Color steelblue{0.275f, 0.510f, 0.706f};
    constexpr Color tan{0.824f, 0.706f, 0.549f};
    constexpr Color teal{0.000f, 0.502f, 0.502f};
    constexpr Color thistle{0.847f, 0.749f, 0.847f};
    constexpr Color tomato{1.000f, 0.388f, 0.278f};
    constexpr Color turquoise{0.251f, 0.878f, 0.816f};
    constexpr Color violet{0.933f, 0.510f, 0.933f};
    constexpr Color wheat{0.961f, 0.871f, 0.702f};
    constexpr Color white{1.000f, 1.000f, 1.000f};
    constexpr Color whitesmoke{0.961f, 0.961f, 0.961f};
    constexpr Color yellow{1.000f, 1.000f, 0.000f};
    constexpr Color yellowgreen{0.604f, 0.804f, 0.196f};
} /* namespace palette */

} /* namespace plot */
