#include <math.h>
#include "allocore/graphics/al_Isosurface.hpp"

namespace al{

// File Name: Isosurface.cpp
// Last Modified: 5/8/2000
// Author: Raghavendra Chandrashekara (based on source code provided
// by Paul Bourke and Cory Gene Bloyd)
// Email: rc99@doc.ic.ac.uk, rchandrashekara@hotmail.com
//
// Description: This is the implementation file for the Isosurface class.


// This table maps an 8-bit mask of the box vertices inside the isosurface
// to a 12-bit mask of edges intersected by the surface.
//template <class T> const short Isosurface<T>::sEdgeTable[256] = {
static const short sEdgeTable[256] = {
0x0  ,0x109,0x203,0x30a,0x406,0x50f,0x605,0x70c,0x80c,0x905,0xa0f,0xb06,0xc0a,0xd03,0xe09,0xf00,
0x190,0x99 ,0x393,0x29a,0x596,0x49f,0x795,0x69c,0x99c,0x895,0xb9f,0xa96,0xd9a,0xc93,0xf99,0xe90,
0x230,0x339,0x33 ,0x13a,0x636,0x73f,0x435,0x53c,0xa3c,0xb35,0x83f,0x936,0xe3a,0xf33,0xc39,0xd30,
0x3a0,0x2a9,0x1a3,0xaa ,0x7a6,0x6af,0x5a5,0x4ac,0xbac,0xaa5,0x9af,0x8a6,0xfaa,0xea3,0xda9,0xca0,
0x460,0x569,0x663,0x76a,0x66 ,0x16f,0x265,0x36c,0xc6c,0xd65,0xe6f,0xf66,0x86a,0x963,0xa69,0xb60,
0x5f0,0x4f9,0x7f3,0x6fa,0x1f6,0xff ,0x3f5,0x2fc,0xdfc,0xcf5,0xfff,0xef6,0x9fa,0x8f3,0xbf9,0xaf0,
0x650,0x759,0x453,0x55a,0x256,0x35f,0x55 ,0x15c,0xe5c,0xf55,0xc5f,0xd56,0xa5a,0xb53,0x859,0x950,
0x7c0,0x6c9,0x5c3,0x4ca,0x3c6,0x2cf,0x1c5,0xcc ,0xfcc,0xec5,0xdcf,0xcc6,0xbca,0xac3,0x9c9,0x8c0,
0x8c0,0x9c9,0xac3,0xbca,0xcc6,0xdcf,0xec5,0xfcc,0xcc ,0x1c5,0x2cf,0x3c6,0x4ca,0x5c3,0x6c9,0x7c0,
0x950,0x859,0xb53,0xa5a,0xd56,0xc5f,0xf55,0xe5c,0x15c,0x55 ,0x35f,0x256,0x55a,0x453,0x759,0x650,
0xaf0,0xbf9,0x8f3,0x9fa,0xef6,0xfff,0xcf5,0xdfc,0x2fc,0x3f5,0xff ,0x1f6,0x6fa,0x7f3,0x4f9,0x5f0,
0xb60,0xa69,0x963,0x86a,0xf66,0xe6f,0xd65,0xc6c,0x36c,0x265,0x16f,0x66 ,0x76a,0x663,0x569,0x460,
0xca0,0xda9,0xea3,0xfaa,0x8a6,0x9af,0xaa5,0xbac,0x4ac,0x5a5,0x6af,0x7a6,0xaa ,0x1a3,0x2a9,0x3a0,
0xd30,0xc39,0xf33,0xe3a,0x936,0x83f,0xb35,0xa3c,0x53c,0x435,0x73f,0x636,0x13a,0x33 ,0x339,0x230,
0xe90,0xf99,0xc93,0xd9a,0xa96,0xb9f,0x895,0x99c,0x69c,0x795,0x49f,0x596,0x29a,0x393,0x99 ,0x190,
0xf00,0xe09,0xd03,0xc0a,0xb06,0xa0f,0x905,0x80c,0x70c,0x605,0x50f,0x406,0x30a,0x203,0x109,0x0   
};

// The first index is an 8-bit mask of box vertices inside isosurface.
// The second index accesses successive vertices of the corners of the triangles
// making up the isosurface according to the first index.
static const char sTriTable[256][16] = {
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,8,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,1,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,8,3,9,8,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,8,3,1,2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{9,2,10,0,2,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{2,8,3,2,10,8,10,9,8,-1,-1,-1,-1,-1,-1,-1},
{3,11,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,11,2,8,11,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,9,0,2,3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,11,2,1,9,11,9,8,11,-1,-1,-1,-1,-1,-1,-1},
{3,10,1,11,10,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,10,1,0,8,10,8,11,10,-1,-1,-1,-1,-1,-1,-1},
{3,9,0,3,11,9,11,10,9,-1,-1,-1,-1,-1,-1,-1},
{9,8,10,10,8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,3,0,7,3,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,1,9,8,4,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,1,9,4,7,1,7,3,1,-1,-1,-1,-1,-1,-1,-1},
{1,2,10,8,4,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,4,7,3,0,4,1,2,10,-1,-1,-1,-1,-1,-1,-1},
{9,2,10,9,0,2,8,4,7,-1,-1,-1,-1,-1,-1,-1},
{2,10,9,2,9,7,2,7,3,7,9,4,-1,-1,-1,-1},
{8,4,7,3,11,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{11,4,7,11,2,4,2,0,4,-1,-1,-1,-1,-1,-1,-1},
{9,0,1,8,4,7,2,3,11,-1,-1,-1,-1,-1,-1,-1},
{4,7,11,9,4,11,9,11,2,9,2,1,-1,-1,-1,-1},
{3,10,1,3,11,10,7,8,4,-1,-1,-1,-1,-1,-1,-1},
{1,11,10,1,4,11,1,0,4,7,11,4,-1,-1,-1,-1},
{4,7,8,9,0,11,9,11,10,11,0,3,-1,-1,-1,-1},
{4,7,11,4,11,9,9,11,10,-1,-1,-1,-1,-1,-1,-1},
{9,5,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{9,5,4,0,8,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,5,4,1,5,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{8,5,4,8,3,5,3,1,5,-1,-1,-1,-1,-1,-1,-1},
{1,2,10,9,5,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,0,8,1,2,10,4,9,5,-1,-1,-1,-1,-1,-1,-1},
{5,2,10,5,4,2,4,0,2,-1,-1,-1,-1,-1,-1,-1},
{2,10,5,3,2,5,3,5,4,3,4,8,-1,-1,-1,-1},
{9,5,4,2,3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,11,2,0,8,11,4,9,5,-1,-1,-1,-1,-1,-1,-1},
{0,5,4,0,1,5,2,3,11,-1,-1,-1,-1,-1,-1,-1},
{2,1,5,2,5,8,2,8,11,4,8,5,-1,-1,-1,-1},
{10,3,11,10,1,3,9,5,4,-1,-1,-1,-1,-1,-1,-1},
{4,9,5,0,8,1,8,10,1,8,11,10,-1,-1,-1,-1},
{5,4,0,5,0,11,5,11,10,11,0,3,-1,-1,-1,-1},
{5,4,8,5,8,10,10,8,11,-1,-1,-1,-1,-1,-1,-1},
{9,7,8,5,7,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{9,3,0,9,5,3,5,7,3,-1,-1,-1,-1,-1,-1,-1},
{0,7,8,0,1,7,1,5,7,-1,-1,-1,-1,-1,-1,-1},
{1,5,3,3,5,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{9,7,8,9,5,7,10,1,2,-1,-1,-1,-1,-1,-1,-1},
{10,1,2,9,5,0,5,3,0,5,7,3,-1,-1,-1,-1},
{8,0,2,8,2,5,8,5,7,10,5,2,-1,-1,-1,-1},
{2,10,5,2,5,3,3,5,7,-1,-1,-1,-1,-1,-1,-1},
{7,9,5,7,8,9,3,11,2,-1,-1,-1,-1,-1,-1,-1},
{9,5,7,9,7,2,9,2,0,2,7,11,-1,-1,-1,-1},
{2,3,11,0,1,8,1,7,8,1,5,7,-1,-1,-1,-1},
{11,2,1,11,1,7,7,1,5,-1,-1,-1,-1,-1,-1,-1},
{9,5,8,8,5,7,10,1,3,10,3,11,-1,-1,-1,-1},
{5,7,0,5,0,9,7,11,0,1,0,10,11,10,0,-1},
{11,10,0,11,0,3,10,5,0,8,0,7,5,7,0,-1},
{11,10,5,7,11,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{10,6,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,8,3,5,10,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{9,0,1,5,10,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,8,3,1,9,8,5,10,6,-1,-1,-1,-1,-1,-1,-1},
{1,6,5,2,6,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,6,5,1,2,6,3,0,8,-1,-1,-1,-1,-1,-1,-1},
{9,6,5,9,0,6,0,2,6,-1,-1,-1,-1,-1,-1,-1},
{5,9,8,5,8,2,5,2,6,3,2,8,-1,-1,-1,-1},
{2,3,11,10,6,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{11,0,8,11,2,0,10,6,5,-1,-1,-1,-1,-1,-1,-1},
{0,1,9,2,3,11,5,10,6,-1,-1,-1,-1,-1,-1,-1},
{5,10,6,1,9,2,9,11,2,9,8,11,-1,-1,-1,-1},
{6,3,11,6,5,3,5,1,3,-1,-1,-1,-1,-1,-1,-1},
{0,8,11,0,11,5,0,5,1,5,11,6,-1,-1,-1,-1},
{3,11,6,0,3,6,0,6,5,0,5,9,-1,-1,-1,-1},
{6,5,9,6,9,11,11,9,8,-1,-1,-1,-1,-1,-1,-1},
{5,10,6,4,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,3,0,4,7,3,6,5,10,-1,-1,-1,-1,-1,-1,-1},
{1,9,0,5,10,6,8,4,7,-1,-1,-1,-1,-1,-1,-1},
{10,6,5,1,9,7,1,7,3,7,9,4,-1,-1,-1,-1},
{6,1,2,6,5,1,4,7,8,-1,-1,-1,-1,-1,-1,-1},
{1,2,5,5,2,6,3,0,4,3,4,7,-1,-1,-1,-1},
{8,4,7,9,0,5,0,6,5,0,2,6,-1,-1,-1,-1},
{7,3,9,7,9,4,3,2,9,5,9,6,2,6,9,-1},
{3,11,2,7,8,4,10,6,5,-1,-1,-1,-1,-1,-1,-1},
{5,10,6,4,7,2,4,2,0,2,7,11,-1,-1,-1,-1},
{0,1,9,4,7,8,2,3,11,5,10,6,-1,-1,-1,-1},
{9,2,1,9,11,2,9,4,11,7,11,4,5,10,6,-1},
{8,4,7,3,11,5,3,5,1,5,11,6,-1,-1,-1,-1},
{5,1,11,5,11,6,1,0,11,7,11,4,0,4,11,-1},
{0,5,9,0,6,5,0,3,6,11,6,3,8,4,7,-1},
{6,5,9,6,9,11,4,7,9,7,11,9,-1,-1,-1,-1},
{10,4,9,6,4,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,10,6,4,9,10,0,8,3,-1,-1,-1,-1,-1,-1,-1},
{10,0,1,10,6,0,6,4,0,-1,-1,-1,-1,-1,-1,-1},
{8,3,1,8,1,6,8,6,4,6,1,10,-1,-1,-1,-1},
{1,4,9,1,2,4,2,6,4,-1,-1,-1,-1,-1,-1,-1},
{3,0,8,1,2,9,2,4,9,2,6,4,-1,-1,-1,-1},
{0,2,4,4,2,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{8,3,2,8,2,4,4,2,6,-1,-1,-1,-1,-1,-1,-1},
{10,4,9,10,6,4,11,2,3,-1,-1,-1,-1,-1,-1,-1},
{0,8,2,2,8,11,4,9,10,4,10,6,-1,-1,-1,-1},
{3,11,2,0,1,6,0,6,4,6,1,10,-1,-1,-1,-1},
{6,4,1,6,1,10,4,8,1,2,1,11,8,11,1,-1},
{9,6,4,9,3,6,9,1,3,11,6,3,-1,-1,-1,-1},
{8,11,1,8,1,0,11,6,1,9,1,4,6,4,1,-1},
{3,11,6,3,6,0,0,6,4,-1,-1,-1,-1,-1,-1,-1},
{6,4,8,11,6,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{7,10,6,7,8,10,8,9,10,-1,-1,-1,-1,-1,-1,-1},
{0,7,3,0,10,7,0,9,10,6,7,10,-1,-1,-1,-1},
{10,6,7,1,10,7,1,7,8,1,8,0,-1,-1,-1,-1},
{10,6,7,10,7,1,1,7,3,-1,-1,-1,-1,-1,-1,-1},
{1,2,6,1,6,8,1,8,9,8,6,7,-1,-1,-1,-1},
{2,6,9,2,9,1,6,7,9,0,9,3,7,3,9,-1},
{7,8,0,7,0,6,6,0,2,-1,-1,-1,-1,-1,-1,-1},
{7,3,2,6,7,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{2,3,11,10,6,8,10,8,9,8,6,7,-1,-1,-1,-1},
{2,0,7,2,7,11,0,9,7,6,7,10,9,10,7,-1},
{1,8,0,1,7,8,1,10,7,6,7,10,2,3,11,-1},
{11,2,1,11,1,7,10,6,1,6,7,1,-1,-1,-1,-1},
{8,9,6,8,6,7,9,1,6,11,6,3,1,3,6,-1},
{0,9,1,11,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{7,8,0,7,0,6,3,11,0,11,6,0,-1,-1,-1,-1},
{7,11,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{7,6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,0,8,11,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,1,9,11,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{8,1,9,8,3,1,11,7,6,-1,-1,-1,-1,-1,-1,-1},
{10,1,2,6,11,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,2,10,3,0,8,6,11,7,-1,-1,-1,-1,-1,-1,-1},
{2,9,0,2,10,9,6,11,7,-1,-1,-1,-1,-1,-1,-1},
{6,11,7,2,10,3,10,8,3,10,9,8,-1,-1,-1,-1},
{7,2,3,6,2,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{7,0,8,7,6,0,6,2,0,-1,-1,-1,-1,-1,-1,-1},
{2,7,6,2,3,7,0,1,9,-1,-1,-1,-1,-1,-1,-1},
{1,6,2,1,8,6,1,9,8,8,7,6,-1,-1,-1,-1},
{10,7,6,10,1,7,1,3,7,-1,-1,-1,-1,-1,-1,-1},
{10,7,6,1,7,10,1,8,7,1,0,8,-1,-1,-1,-1},
{0,3,7,0,7,10,0,10,9,6,10,7,-1,-1,-1,-1},
{7,6,10,7,10,8,8,10,9,-1,-1,-1,-1,-1,-1,-1},
{6,8,4,11,8,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,6,11,3,0,6,0,4,6,-1,-1,-1,-1,-1,-1,-1},
{8,6,11,8,4,6,9,0,1,-1,-1,-1,-1,-1,-1,-1},
{9,4,6,9,6,3,9,3,1,11,3,6,-1,-1,-1,-1},
{6,8,4,6,11,8,2,10,1,-1,-1,-1,-1,-1,-1,-1},
{1,2,10,3,0,11,0,6,11,0,4,6,-1,-1,-1,-1},
{4,11,8,4,6,11,0,2,9,2,10,9,-1,-1,-1,-1},
{10,9,3,10,3,2,9,4,3,11,3,6,4,6,3,-1},
{8,2,3,8,4,2,4,6,2,-1,-1,-1,-1,-1,-1,-1},
{0,4,2,4,6,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,9,0,2,3,4,2,4,6,4,3,8,-1,-1,-1,-1},
{1,9,4,1,4,2,2,4,6,-1,-1,-1,-1,-1,-1,-1},
{8,1,3,8,6,1,8,4,6,6,10,1,-1,-1,-1,-1},
{10,1,0,10,0,6,6,0,4,-1,-1,-1,-1,-1,-1,-1},
{4,6,3,4,3,8,6,10,3,0,3,9,10,9,3,-1},
{10,9,4,6,10,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,9,5,7,6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,8,3,4,9,5,11,7,6,-1,-1,-1,-1,-1,-1,-1},
{5,0,1,5,4,0,7,6,11,-1,-1,-1,-1,-1,-1,-1},
{11,7,6,8,3,4,3,5,4,3,1,5,-1,-1,-1,-1},
{9,5,4,10,1,2,7,6,11,-1,-1,-1,-1,-1,-1,-1},
{6,11,7,1,2,10,0,8,3,4,9,5,-1,-1,-1,-1},
{7,6,11,5,4,10,4,2,10,4,0,2,-1,-1,-1,-1},
{3,4,8,3,5,4,3,2,5,10,5,2,11,7,6,-1},
{7,2,3,7,6,2,5,4,9,-1,-1,-1,-1,-1,-1,-1},
{9,5,4,0,8,6,0,6,2,6,8,7,-1,-1,-1,-1},
{3,6,2,3,7,6,1,5,0,5,4,0,-1,-1,-1,-1},
{6,2,8,6,8,7,2,1,8,4,8,5,1,5,8,-1},
{9,5,4,10,1,6,1,7,6,1,3,7,-1,-1,-1,-1},
{1,6,10,1,7,6,1,0,7,8,7,0,9,5,4,-1},
{4,0,10,4,10,5,0,3,10,6,10,7,3,7,10,-1},
{7,6,10,7,10,8,5,4,10,4,8,10,-1,-1,-1,-1},
{6,9,5,6,11,9,11,8,9,-1,-1,-1,-1,-1,-1,-1},
{3,6,11,0,6,3,0,5,6,0,9,5,-1,-1,-1,-1},
{0,11,8,0,5,11,0,1,5,5,6,11,-1,-1,-1,-1},
{6,11,3,6,3,5,5,3,1,-1,-1,-1,-1,-1,-1,-1},
{1,2,10,9,5,11,9,11,8,11,5,6,-1,-1,-1,-1},
{0,11,3,0,6,11,0,9,6,5,6,9,1,2,10,-1},
{11,8,5,11,5,6,8,0,5,10,5,2,0,2,5,-1},
{6,11,3,6,3,5,2,10,3,10,5,3,-1,-1,-1,-1},
{5,8,9,5,2,8,5,6,2,3,8,2,-1,-1,-1,-1},
{9,5,6,9,6,0,0,6,2,-1,-1,-1,-1,-1,-1,-1},
{1,5,8,1,8,0,5,6,8,3,8,2,6,2,8,-1},
{1,5,6,2,1,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,3,6,1,6,10,3,8,6,5,6,9,8,9,6,-1},
{10,1,0,10,0,6,9,5,0,5,6,0,-1,-1,-1,-1},
{0,3,8,5,6,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{10,5,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{11,5,10,7,5,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{11,5,10,11,7,5,8,3,0,-1,-1,-1,-1,-1,-1,-1},
{5,11,7,5,10,11,1,9,0,-1,-1,-1,-1,-1,-1,-1},
{10,7,5,10,11,7,9,8,1,8,3,1,-1,-1,-1,-1},
{11,1,2,11,7,1,7,5,1,-1,-1,-1,-1,-1,-1,-1},
{0,8,3,1,2,7,1,7,5,7,2,11,-1,-1,-1,-1},
{9,7,5,9,2,7,9,0,2,2,11,7,-1,-1,-1,-1},
{7,5,2,7,2,11,5,9,2,3,2,8,9,8,2,-1},
{2,5,10,2,3,5,3,7,5,-1,-1,-1,-1,-1,-1,-1},
{8,2,0,8,5,2,8,7,5,10,2,5,-1,-1,-1,-1},
{9,0,1,5,10,3,5,3,7,3,10,2,-1,-1,-1,-1},
{9,8,2,9,2,1,8,7,2,10,2,5,7,5,2,-1},
{1,3,5,3,7,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,8,7,0,7,1,1,7,5,-1,-1,-1,-1,-1,-1,-1},
{9,0,3,9,3,5,5,3,7,-1,-1,-1,-1,-1,-1,-1},
{9,8,7,5,9,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{5,8,4,5,10,8,10,11,8,-1,-1,-1,-1,-1,-1,-1},
{5,0,4,5,11,0,5,10,11,11,3,0,-1,-1,-1,-1},
{0,1,9,8,4,10,8,10,11,10,4,5,-1,-1,-1,-1},
{10,11,4,10,4,5,11,3,4,9,4,1,3,1,4,-1},
{2,5,1,2,8,5,2,11,8,4,5,8,-1,-1,-1,-1},
{0,4,11,0,11,3,4,5,11,2,11,1,5,1,11,-1},
{0,2,5,0,5,9,2,11,5,4,5,8,11,8,5,-1},
{9,4,5,2,11,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{2,5,10,3,5,2,3,4,5,3,8,4,-1,-1,-1,-1},
{5,10,2,5,2,4,4,2,0,-1,-1,-1,-1,-1,-1,-1},
{3,10,2,3,5,10,3,8,5,4,5,8,0,1,9,-1},
{5,10,2,5,2,4,1,9,2,9,4,2,-1,-1,-1,-1},
{8,4,5,8,5,3,3,5,1,-1,-1,-1,-1,-1,-1,-1},
{0,4,5,1,0,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{8,4,5,8,5,3,9,0,5,0,3,5,-1,-1,-1,-1},
{9,4,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,11,7,4,9,11,9,10,11,-1,-1,-1,-1,-1,-1,-1},
{0,8,3,4,9,7,9,11,7,9,10,11,-1,-1,-1,-1},
{1,10,11,1,11,4,1,4,0,7,4,11,-1,-1,-1,-1},
{3,1,4,3,4,8,1,10,4,7,4,11,10,11,4,-1},
{4,11,7,9,11,4,9,2,11,9,1,2,-1,-1,-1,-1},
{9,7,4,9,11,7,9,1,11,2,11,1,0,8,3,-1},
{11,7,4,11,4,2,2,4,0,-1,-1,-1,-1,-1,-1,-1},
{11,7,4,11,4,2,8,3,4,3,2,4,-1,-1,-1,-1},
{2,9,10,2,7,9,2,3,7,7,4,9,-1,-1,-1,-1},
{9,10,7,9,7,4,10,2,7,8,7,0,2,0,7,-1},
{3,7,10,3,10,2,7,4,10,1,10,0,4,0,10,-1},
{1,10,2,8,7,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,9,1,4,1,7,7,1,3,-1,-1,-1,-1,-1,-1,-1},
{4,9,1,4,1,7,0,8,1,8,7,1,-1,-1,-1,-1},
{4,0,3,7,4,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{4,8,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{9,10,8,10,11,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,0,9,3,9,11,11,9,10,-1,-1,-1,-1,-1,-1,-1},
{0,1,10,0,10,8,8,10,11,-1,-1,-1,-1,-1,-1,-1},
{3,1,10,11,3,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,2,11,1,11,9,9,11,8,-1,-1,-1,-1,-1,-1,-1},
{3,0,9,3,9,11,1,2,9,2,11,9,-1,-1,-1,-1},
{0,2,11,8,0,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{2,3,8,2,8,10,10,8,9,-1,-1,-1,-1,-1,-1,-1},
{9,10,2,0,9,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{2,3,8,2,8,10,0,1,8,1,10,8,-1,-1,-1,-1},
{1,10,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{1,3,8,9,1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,9,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{0,3,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

/*
static char edgeToVertices[12][2] = {
	{0,1}, {1,2}, {2,3}, {3,0},
	{4,5}, {5,6}, {6,7}, {7,4},
	{0,4}, {1,5}, {2,6}, {3,7}
};	
*/

template <class T> Isosurface<T>::Isosurface(const T& isolev)
:	mN1(0), mN2(0), mN3(0), mN1p(0), mN2p(0), mL1(0), mL2(0), mL3(0),
	mIsolevel(isolev)
{
	mValidSurface = false;
}

template <class T> Isosurface<T>::~Isosurface(){
	clear();
}

template <class T>
void Isosurface<T>::addCell(const T * cellVals, const int * cellIdx3){
	const int &ix = cellIdx3[0];
	const int &iy = cellIdx3[1];
	const int &iz = cellIdx3[2];

	// Get isosurface cell index depending on field values at corners of cell
	int idx = 0;
	if(cellVals[0] < level()) idx |=   1;
	if(cellVals[2] < level()) idx |=   2;
	if(cellVals[3] < level()) idx |=   4;
	if(cellVals[1] < level()) idx |=   8;
	if(cellVals[4] < level()) idx |=  16;
	if(cellVals[6] < level()) idx |=  32;
	if(cellVals[7] < level()) idx |=  64;
	if(cellVals[5] < level()) idx |= 128;

	// Create a triangulation of the isosurface in this cell.
	int edgeCode = sEdgeTable[idx];
	if(edgeCode){
	
		int vID = vertexID(ix,iy,iz);

		// Compute interpolated vertices on edges of box
		//		(int EdgeID, PointID vertex) pair is created in mEdgeToVertex
		//		EdgeID is a unique integer ID of box edge
		//		vertex.newID is not set to anything
		if(edgeCode &    1) addEdgeVertex(ix,iy,iz,vID, 0, cellVals);
		if(edgeCode &    2) addEdgeVertex(ix,iy,iz,vID, 1, cellVals);
		if(edgeCode &    4) addEdgeVertex(ix,iy,iz,vID, 2, cellVals);
		if(edgeCode &    8) addEdgeVertex(ix,iy,iz,vID, 3, cellVals);
		if(edgeCode &   16) addEdgeVertex(ix,iy,iz,vID, 4, cellVals);
		if(edgeCode &   32) addEdgeVertex(ix,iy,iz,vID, 5, cellVals);
		if(edgeCode &   64) addEdgeVertex(ix,iy,iz,vID, 6, cellVals);
		if(edgeCode &  128) addEdgeVertex(ix,iy,iz,vID, 7, cellVals);
		if(edgeCode &  256) addEdgeVertex(ix,iy,iz,vID, 8, cellVals);
		if(edgeCode &  512) addEdgeVertex(ix,iy,iz,vID, 9, cellVals);
		if(edgeCode & 1024) addEdgeVertex(ix,iy,iz,vID,10, cellVals);
		if(edgeCode & 2048) addEdgeVertex(ix,iy,iz,vID,11, cellVals);

		// Add up to 5 triangles (15 vertices) representing surface through cell
		// Each triangle consists of 3 vertex indices stored in mEdgeToVertex
		for(int i=0; sTriTable[idx][i] != -1; i+=3){
			
			// Add 3 indices of triangle
			EdgeTriangle triangle;
			triangle.edgeIDs[0] = edgeID(vID, sTriTable[idx][i  ]);
			triangle.edgeIDs[1] = edgeID(vID, sTriTable[idx][i+1]);
			triangle.edgeIDs[2] = edgeID(vID, sTriTable[idx][i+2]);
			mEdgeTriangles.append(triangle, 1.5);
		}
	}
}


template <class T>
void Isosurface<T>::addEdgeVertex(int ix, int iy, int iz, int vertID, int edgeNo, const T * cellVals){

	int id = edgeID(vertID, edgeNo);

	typename EdgeToVertex::iterator it = mEdgeToVertex.find(id);
	
	// If this edge vertex has not been computed yet, then compute it
	if(it == mEdgeToVertex.end()){
		PointID pt = calcIntersection(ix,iy,iz, edgeNo, cellVals);
		mEdgeToVertex[id] = pt;
	}
};


template <class T>
typename Isosurface<T>::PointID 
Isosurface<T>::calcIntersection(int ix, int iy, int iz, int edgeNo, const T * cellVals) const{

	// Positions of cube vertices
	static const int cubeVertices[8][3] = {
	/*	0        1        2        3        4        5        6        7 */
		{0,0,0}, {1,0,0}, {0,1,0}, {1,1,0}, {0,0,1}, {1,0,1}, {0,1,1}, {1,1,1}
	};

	// Store differences between cube vertices for faster interpolation
	static const float edgeDirections[12][3] = {
		{0,1,0}, {1,0,0}, {0,-1,0}, {-1,0,0},
		{0,1,0}, {1,0,0}, {0,-1,0}, {-1,0,0},
		{0,0,1}, {0,0,1}, {0, 0,1}, { 0,0,1}
	};

	// Edge number to cell corners map
	static const char cellEdgeIndices[12][2] = {
		{0,2}, {2,3}, {3,1}, {1,0},
		{4,6}, {6,7}, {7,5}, {5,4},
		{0,4}, {2,6}, {3,7}, {1,5}
	};

	const char& i0 = cellEdgeIndices[edgeNo][0];
	const char& i1 = cellEdgeIndices[edgeNo][1];
	const T& val1 = cellVals[i0];
	const T& val2 = cellVals[i1];
	const int * e1 = cubeVertices[i0];
	const float * ed = edgeDirections[edgeNo];

	// Interpolate between two grid points to produce the point at which
	// the isosurface intersects an edge.
	
	// 'mu' is the fraction along the edge where the vertex lies
	float mu = float((level() - val1)/(val2 - val1));

	PointID r;
	r.x = (ix + e1[0] + mu*ed[0]) * mL1;
	r.y = (iy + e1[1] + mu*ed[1]) * mL2;
	r.z = (iz + e1[2] + mu*ed[2]) * mL3;

	return r;
}


template <class T>
void Isosurface<T>::begin(){
	mValidSurface = false;
}


template <class T>
void Isosurface<T>::end(){
	reset();
	compressTriangles();
	generateNormals();
//	calcNormals();
	mValidSurface = true;
}


template <class T> 
int Isosurface<T>::edgeID(int vertID, int edgeNo) const{
	return vertID + mEdgeIDOffsets[edgeNo];
}

template <class T>
int Isosurface<T>::vertexID(int ix, int iy, int iz) const{
	return 3*(ix + mN1p * (iy + mN2p * iz));
}

template <class T>
void Isosurface<T>::generate(
	const T* field, int nX, int nY, int nZ,
	float cellLengthX, float cellLengthY, float cellLengthZ
){
	begin();

	cellDims(nX, nY, nZ);
	cellLengths(cellLengthX, cellLengthY, cellLengthZ);

	// center surface
	//pos(Vec3d(-1,-1,-1));

	int Nx = nX;
	int Nxy = Nx*nY;

	// Generate isosurface.
	for(int z=0; z < mN3; ++z){
		int z0 = z   *Nxy;
		int z1 =(z+1)*Nxy;
	for(int y=0; y < mN2; ++y){
		int y0 = y   *Nx;
		int y1 =(y+1)*Nx;
	for(int x=0; x < mN1; ++x){

		int x0 = x;
		int x1 =(x+1);

		addCell(
			field[z0 + y0 + x0], field[z0 + y0 + x1],
			field[z0 + y1 + x0], field[z0 + y1 + x1],
			field[z1 + y0 + x0], field[z1 + y0 + x1],
			field[z1 + y1 + x0], field[z1 + y1 + x1],
			x,y,z
		);

	}}}
	
	end();
}


template <class T>
int Isosurface<T>::volumeLengths(double& volLengthX, double& volLengthY, double& volLengthZ) const {
	if(validSurface()){
		volLengthX = mL1*mN1;
		volLengthY = mL2*mN2;
		volLengthZ = mL3*mN3;
		return 1;
	}
	return -1;
}


// Compress vertices and triangles so that they can be accessed more efficiently
template <class T>
void Isosurface<T>::compressTriangles(){

	// We have:
	// mEdgeToVertex-	map of (edge id, vertex) pairs making up surface
	//					vertices lie on edges of cells
	// mEdgeTriangles-		array of indices specifying triangles
	//					every 3 indices is a triangle

//	struct PointID { int newID; double x, y, z; };
//	struct EdgeTriangle{ int edgeIDs[3]; };

	// Assign edge vertices to sequential vertex buffer locations
	{
		int nextID = -1;
		typename EdgeToVertex::iterator it = mEdgeToVertex.begin();
		for(; it != mEdgeToVertex.end(); ++it){
			PointID& pt = it->second;
			pt.newID = ++nextID;		// store vertex index
			vertex(pt.x, pt.y, pt.z);	// add vertex to vertex buffer
		}
	}

	// Reassign triangles to vertex buffer locations
	for(int i=0; i<mEdgeTriangles.size(); ++i){
		const EdgeTriangle& tri = mEdgeTriangles[i];

		for(int j=0; j<3; ++j){
			int newID = mEdgeToVertex[tri.edgeIDs[j]].newID;
			
			// Add index to vertex indices buffer
			index(newID);
		}		
	}

	mEdgeToVertex.clear();
	mEdgeTriangles.reset();
}


//template <class T>
//void Isosurface<T>::calcNormals(){
//	mNormals.size(mVertices.size());
//	mNormals.assign(mNormals.size(), Vec3f(0,0,0));
//
//	// Calculate normals
//	// The vertex normals are a sum of all neighboring triangles normals
//	for(int i=0; i<mIndices.size(); i+=3){
//
//		// Get indices of triangle vertices
//		int i0 = mIndices[i  ];
//		int i1 = mIndices[i+1];
//		int i2 = mIndices[i+2];
//		
//		// Compute normal of triangle
//		Mesh::Vertex v1 = mVertices[i1] - mVertices[i0];
//		Mesh::Vertex v2 = mVertices[i2] - mVertices[i0];
//		Mesh::Vertex nd = v1 ^ v2;
//
//		mNormals[i0] += nd;
//		mNormals[i1] += nd;
//		mNormals[i2] += nd;
//	}
//
//	// Normalize normals
//	for(int i=0; i < mNormals.size(); i++){
//		mNormals[i].normalize();
//	}
//}



template <class T>
Isosurface<T>& Isosurface<T>::cellDims(int nx, int ny, int nz){
	mN1p=nx;
	mN2p=ny;
	mN1 = nx-1;		// these are less 1 since the other corner of the cell is (x+1, y+1, z+1)
	mN2 = ny-1;
	mN3 = nz-1;
	
	mEdgeIDOffsets[ 0] = 1;
	mEdgeIDOffsets[ 1] = 0 + 3*mN1p;
	mEdgeIDOffsets[ 2] = 1 + 3;
	mEdgeIDOffsets[ 3] = 0;
	mEdgeIDOffsets[ 4] = 1 + 3*mN1p*mN2p;
	mEdgeIDOffsets[ 5] = 0 + 3*(mN1p + mN1p*mN2p);
	mEdgeIDOffsets[ 6] = 1 + 3*(1 + mN1p*mN2p);
	mEdgeIDOffsets[ 7] = 0 + 3*mN1p*mN2p;
	mEdgeIDOffsets[ 8] = 2;
	mEdgeIDOffsets[ 9] = 2 + 3*mN1p;
	mEdgeIDOffsets[10] = 2 + 3*(1+mN1p);
	mEdgeIDOffsets[11] = 2 + 3;
	return *this;
}


template <class T>
Isosurface<T>& Isosurface<T>::cellLengths(double dx, double dy, double dz){
	mL1=dx; mL2=dy; mL3=dz;
	return *this;
}


template <class T>
void Isosurface<T>::clear(){
	mL1 = mL2 = mL3 = mN1 = mN2 = mN3 = mN1p = mN2p = 0;
	mValidSurface = false;
}



// Define some templates so implementation can stay in .cpp
//template class Isosurface<short>;
//template class Isosurface<unsigned short>;
template class Isosurface<float>;
template class Isosurface<double>;

} // al::

/*

Biggest hit is lookup into the edge-to-vertex map.
Can we just use a counter for edge IDs?

12.9%	12.9%	isosurface	al::Isosurface<float>::addCell(float const*, int const*)
12.7%	12.7%	isosurface	std::tr1::hashtable<int, std::pair<int const, al::Isosurface<float>::PointID>, std::allocator<std::pair<int const, al::Isosurface<float>::PointID> >, Internal::extract1st<std::pair<int const, al::Isosurface<float>::PointID> >, std::equal_to<int>, al::Isosurface<float>::IsosurfaceHashInt, Internal::mod_range_hashing, Internal::default_ranged_hash, Internal::prime_rehash_policy, false, true, true>::insert(std::pair<int const, al::Isosurface<float>::PointID> const&, std::tr1::integral_constant<bool, true>)
0.0%	9.9%	isosurface		al::Isosurface<float>::compressTriangles()
0.0%	2.6%	isosurface		al::Isosurface<float>::addEdgeVertex(int, int, int, int, int, float const*)
0.0%	0.1%	isosurface		al::Isosurface<float>::generate(float const*, int, int, int, float, float, float)
9.0%	9.0%	isosurface	al::Isosurface<float>::compressTriangles()
7.8%	7.8%	libSystem.B.dylib	szone_free
7.5%	7.5%	isosurface	al::Mesh::generateNormals(float)
7.1%	7.1%	isosurface	al::Isosurface<float>::generate(float const*, int, int, int, float, float, float)
6.4%	6.4%	isosurface	std::tr1::hashtable<int, std::pair<int const, al::Isosurface<float>::PointID>, std::allocator<std::pair<int const, al::Isosurface<float>::PointID> >, Internal::extract1st<std::pair<int const, al::Isosurface<float>::PointID> >, std::equal_to<int>, al::Isosurface<float>::IsosurfaceHashInt, Internal::mod_range_hashing, Internal::default_ranged_hash, Internal::prime_rehash_policy, false, true, true>::find(int const&)
0.0%	6.3%	isosurface		al::Isosurface<float>::addEdgeVertex(int, int, int, int, int, float const*)
0.0%	0.2%	isosurface		al::Isosurface<float>::addCell(float const*, int const*)
3.7%	3.7%	libSystem.B.dylib	tiny_malloc_from_free_list
3.2%	3.2%	isosurface	al::Isosurface<float>::calcIntersection(int, int, int, int, float const*) const
3.2%	3.2%	isosurface	al::Isosurface<float>::addEdgeVertex(int, int, int, int, int, float const*)


struct CellVertex{
	float value;
	Color color;
	Index index;
};

*/
