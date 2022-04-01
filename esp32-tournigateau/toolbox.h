#ifndef TOOLBOX_H
#define TOOLBOX_H

namespace toolbox {

// ----------------------------------
// const - types
// ----------------------------------

// ----------------------------------
// variables
// ----------------------------------

// ----------------------------------
// services
// ----------------------------------

float clamp(float x, float a, float b);

float lerp(float x, float from, float to);

float unlerp(float l, float from, float to);

float squish(float x, float xFrom, float xTo, float outFrom, float outTo);

};  // namespace toolbox

#endif