#include "toolbox.h"

namespace toolbox {

// ----------------------------------
// const - types
// ----------------------------------

// ----------------------------------
// local variable
// ----------------------------------

// ----------------------------------
// local services
// ----------------------------------

// ----------------------------------
// public services
// ----------------------------------

float clamp(float x, float a, float b) {
    float min = a;
    float max = b;
    if (a > b) {
        min = b;
        max = a;
    }
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

float lerp(float x, float from, float to) {
    return from + x * (to - from);
}

float unlerp(float l, float from, float to) {
    return (l - from) / (to - from);
}

float squish(float x, float xFrom, float xTo, float outFrom, float outTo) {
    if (outFrom == outTo)
        return outTo;
    float l;
    if (xFrom != xTo) {
        l = unlerp(x, xFrom, xTo);
    } else {
        l = 0.5;
    }
    l = lerp(l, outFrom, outTo);
    return clamp(l, outFrom, outTo);
}

}  // namespace toolbox