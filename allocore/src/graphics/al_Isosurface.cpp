#include <cmath>
#include "allocore/types/al_Voxels.hpp"
#include "allocore/graphics/al_Isosurface.hpp"

namespace al{

/*
	 6 --------  7
	 /|       /|
    / |      / |
 2 |--------|3 |
   | 4|-----|--| 5
   | /      |  /
   |/       | /
 0 |--------|1


  +y    +z
  |   /
  | /
  ----- +x

*/

// This table maps an 8-bit mask of the box vertices inside the isosurface
// to a 12-bit mask of edges intersected by the surface.

// For any edge, if one vertex is inside of the surface and the other is outside of the surface
//  then the edge intersects the surface
// For each of the 8 vertices of the cube can be two possible states : either inside or outside of the surface
// For any cube the are 2^8=256 possible sets of vertex states
// This table lists the edges intersected by the surface for all 256 possible vertex states
// There are 12 edges.  For each entry in the table, if edge #n is intersected, then bit #n is set to 1
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
// The second index is an array of triangle indices making up the isosurface
// according to the first index. The first element of the array is its size and
// the following elements are the indices of the triangle in term of edge ID.
static const char sTriTable[256][16] = {
{ 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,0,8,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,0,1,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,1,8,3,9,8,1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,1,2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,0,8,3,1,2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,9,2,10,0,2,9,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,2,8,3,2,10,8,10,9,8,-1,-1,-1,-1,-1,-1},
{ 3,3,11,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,0,11,2,8,11,0,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,1,9,0,2,3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,1,11,2,1,9,11,9,8,11,-1,-1,-1,-1,-1,-1},
{ 6,3,10,1,11,10,3,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,0,10,1,0,8,10,8,11,10,-1,-1,-1,-1,-1,-1},
{ 9,3,9,0,3,11,9,11,10,9,-1,-1,-1,-1,-1,-1},
{ 6,9,8,10,10,8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,4,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,4,3,0,7,3,4,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,0,1,9,8,4,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,4,1,9,4,7,1,7,3,1,-1,-1,-1,-1,-1,-1},
{ 6,1,2,10,8,4,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,3,4,7,3,0,4,1,2,10,-1,-1,-1,-1,-1,-1},
{ 9,9,2,10,9,0,2,8,4,7,-1,-1,-1,-1,-1,-1},
{12,2,10,9,2,9,7,2,7,3,7,9,4,-1,-1,-1},
{ 6,8,4,7,3,11,2,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,11,4,7,11,2,4,2,0,4,-1,-1,-1,-1,-1,-1},
{ 9,9,0,1,8,4,7,2,3,11,-1,-1,-1,-1,-1,-1},
{12,4,7,11,9,4,11,9,11,2,9,2,1,-1,-1,-1},
{ 9,3,10,1,3,11,10,7,8,4,-1,-1,-1,-1,-1,-1},
{12,1,11,10,1,4,11,1,0,4,7,11,4,-1,-1,-1},
{12,4,7,8,9,0,11,9,11,10,11,0,3,-1,-1,-1},
{ 9,4,7,11,4,11,9,9,11,10,-1,-1,-1,-1,-1,-1},
{ 3,9,5,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,9,5,4,0,8,3,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,0,5,4,1,5,0,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,8,5,4,8,3,5,3,1,5,-1,-1,-1,-1,-1,-1},
{ 6,1,2,10,9,5,4,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,3,0,8,1,2,10,4,9,5,-1,-1,-1,-1,-1,-1},
{ 9,5,2,10,5,4,2,4,0,2,-1,-1,-1,-1,-1,-1},
{12,2,10,5,3,2,5,3,5,4,3,4,8,-1,-1,-1},
{ 6,9,5,4,2,3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,0,11,2,0,8,11,4,9,5,-1,-1,-1,-1,-1,-1},
{ 9,0,5,4,0,1,5,2,3,11,-1,-1,-1,-1,-1,-1},
{12,2,1,5,2,5,8,2,8,11,4,8,5,-1,-1,-1},
{ 9,10,3,11,10,1,3,9,5,4,-1,-1,-1,-1,-1,-1},
{12,4,9,5,0,8,1,8,10,1,8,11,10,-1,-1,-1},
{12,5,4,0,5,0,11,5,11,10,11,0,3,-1,-1,-1},
{ 9,5,4,8,5,8,10,10,8,11,-1,-1,-1,-1,-1,-1},
{ 6,9,7,8,5,7,9,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,9,3,0,9,5,3,5,7,3,-1,-1,-1,-1,-1,-1},
{ 9,0,7,8,0,1,7,1,5,7,-1,-1,-1,-1,-1,-1},
{ 6,1,5,3,3,5,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,9,7,8,9,5,7,10,1,2,-1,-1,-1,-1,-1,-1},
{12,10,1,2,9,5,0,5,3,0,5,7,3,-1,-1,-1},
{12,8,0,2,8,2,5,8,5,7,10,5,2,-1,-1,-1},
{ 9,2,10,5,2,5,3,3,5,7,-1,-1,-1,-1,-1,-1},
{ 9,7,9,5,7,8,9,3,11,2,-1,-1,-1,-1,-1,-1},
{12,9,5,7,9,7,2,9,2,0,2,7,11,-1,-1,-1},
{12,2,3,11,0,1,8,1,7,8,1,5,7,-1,-1,-1},
{ 9,11,2,1,11,1,7,7,1,5,-1,-1,-1,-1,-1,-1},
{12,9,5,8,8,5,7,10,1,3,10,3,11,-1,-1,-1},
{15,5,7,0,5,0,9,7,11,0,1,0,10,11,10,0},
{15,11,10,0,11,0,3,10,5,0,8,0,7,5,7,0},
{ 6,11,10,5,7,11,5,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,10,6,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,0,8,3,5,10,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,9,0,1,5,10,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,1,8,3,1,9,8,5,10,6,-1,-1,-1,-1,-1,-1},
{ 6,1,6,5,2,6,1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,1,6,5,1,2,6,3,0,8,-1,-1,-1,-1,-1,-1},
{ 9,9,6,5,9,0,6,0,2,6,-1,-1,-1,-1,-1,-1},
{12,5,9,8,5,8,2,5,2,6,3,2,8,-1,-1,-1},
{ 6,2,3,11,10,6,5,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,11,0,8,11,2,0,10,6,5,-1,-1,-1,-1,-1,-1},
{ 9,0,1,9,2,3,11,5,10,6,-1,-1,-1,-1,-1,-1},
{12,5,10,6,1,9,2,9,11,2,9,8,11,-1,-1,-1},
{ 9,6,3,11,6,5,3,5,1,3,-1,-1,-1,-1,-1,-1},
{12,0,8,11,0,11,5,0,5,1,5,11,6,-1,-1,-1},
{12,3,11,6,0,3,6,0,6,5,0,5,9,-1,-1,-1},
{ 9,6,5,9,6,9,11,11,9,8,-1,-1,-1,-1,-1,-1},
{ 6,5,10,6,4,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,4,3,0,4,7,3,6,5,10,-1,-1,-1,-1,-1,-1},
{ 9,1,9,0,5,10,6,8,4,7,-1,-1,-1,-1,-1,-1},
{12,10,6,5,1,9,7,1,7,3,7,9,4,-1,-1,-1},
{ 9,6,1,2,6,5,1,4,7,8,-1,-1,-1,-1,-1,-1},
{12,1,2,5,5,2,6,3,0,4,3,4,7,-1,-1,-1},
{12,8,4,7,9,0,5,0,6,5,0,2,6,-1,-1,-1},
{15,7,3,9,7,9,4,3,2,9,5,9,6,2,6,9},
{ 9,3,11,2,7,8,4,10,6,5,-1,-1,-1,-1,-1,-1},
{12,5,10,6,4,7,2,4,2,0,2,7,11,-1,-1,-1},
{12,0,1,9,4,7,8,2,3,11,5,10,6,-1,-1,-1},
{15,9,2,1,9,11,2,9,4,11,7,11,4,5,10,6},
{12,8,4,7,3,11,5,3,5,1,5,11,6,-1,-1,-1},
{15,5,1,11,5,11,6,1,0,11,7,11,4,0,4,11},
{15,0,5,9,0,6,5,0,3,6,11,6,3,8,4,7},
{12,6,5,9,6,9,11,4,7,9,7,11,9,-1,-1,-1},
{ 6,10,4,9,6,4,10,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,4,10,6,4,9,10,0,8,3,-1,-1,-1,-1,-1,-1},
{ 9,10,0,1,10,6,0,6,4,0,-1,-1,-1,-1,-1,-1},
{12,8,3,1,8,1,6,8,6,4,6,1,10,-1,-1,-1},
{ 9,1,4,9,1,2,4,2,6,4,-1,-1,-1,-1,-1,-1},
{12,3,0,8,1,2,9,2,4,9,2,6,4,-1,-1,-1},
{ 6,0,2,4,4,2,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,8,3,2,8,2,4,4,2,6,-1,-1,-1,-1,-1,-1},
{ 9,10,4,9,10,6,4,11,2,3,-1,-1,-1,-1,-1,-1},
{12,0,8,2,2,8,11,4,9,10,4,10,6,-1,-1,-1},
{12,3,11,2,0,1,6,0,6,4,6,1,10,-1,-1,-1},
{15,6,4,1,6,1,10,4,8,1,2,1,11,8,11,1},
{12,9,6,4,9,3,6,9,1,3,11,6,3,-1,-1,-1},
{15,8,11,1,8,1,0,11,6,1,9,1,4,6,4,1},
{ 9,3,11,6,3,6,0,0,6,4,-1,-1,-1,-1,-1,-1},
{ 6,6,4,8,11,6,8,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,7,10,6,7,8,10,8,9,10,-1,-1,-1,-1,-1,-1},
{12,0,7,3,0,10,7,0,9,10,6,7,10,-1,-1,-1},
{12,10,6,7,1,10,7,1,7,8,1,8,0,-1,-1,-1},
{ 9,10,6,7,10,7,1,1,7,3,-1,-1,-1,-1,-1,-1},
{12,1,2,6,1,6,8,1,8,9,8,6,7,-1,-1,-1},
{15,2,6,9,2,9,1,6,7,9,0,9,3,7,3,9},
{ 9,7,8,0,7,0,6,6,0,2,-1,-1,-1,-1,-1,-1},
{ 6,7,3,2,6,7,2,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{12,2,3,11,10,6,8,10,8,9,8,6,7,-1,-1,-1},
{15,2,0,7,2,7,11,0,9,7,6,7,10,9,10,7},
{15,1,8,0,1,7,8,1,10,7,6,7,10,2,3,11},
{12,11,2,1,11,1,7,10,6,1,6,7,1,-1,-1,-1},
{15,8,9,6,8,6,7,9,1,6,11,6,3,1,3,6},
{ 6,0,9,1,11,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{12,7,8,0,7,0,6,3,11,0,11,6,0,-1,-1,-1},
{ 3,7,11,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,7,6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,3,0,8,11,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,0,1,9,11,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,8,1,9,8,3,1,11,7,6,-1,-1,-1,-1,-1,-1},
{ 6,10,1,2,6,11,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,1,2,10,3,0,8,6,11,7,-1,-1,-1,-1,-1,-1},
{ 9,2,9,0,2,10,9,6,11,7,-1,-1,-1,-1,-1,-1},
{12,6,11,7,2,10,3,10,8,3,10,9,8,-1,-1,-1},
{ 6,7,2,3,6,2,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,7,0,8,7,6,0,6,2,0,-1,-1,-1,-1,-1,-1},
{ 9,2,7,6,2,3,7,0,1,9,-1,-1,-1,-1,-1,-1},
{12,1,6,2,1,8,6,1,9,8,8,7,6,-1,-1,-1},
{ 9,10,7,6,10,1,7,1,3,7,-1,-1,-1,-1,-1,-1},
{12,10,7,6,1,7,10,1,8,7,1,0,8,-1,-1,-1},
{12,0,3,7,0,7,10,0,10,9,6,10,7,-1,-1,-1},
{ 9,7,6,10,7,10,8,8,10,9,-1,-1,-1,-1,-1,-1},
{ 6,6,8,4,11,8,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,3,6,11,3,0,6,0,4,6,-1,-1,-1,-1,-1,-1},
{ 9,8,6,11,8,4,6,9,0,1,-1,-1,-1,-1,-1,-1},
{12,9,4,6,9,6,3,9,3,1,11,3,6,-1,-1,-1},
{ 9,6,8,4,6,11,8,2,10,1,-1,-1,-1,-1,-1,-1},
{12,1,2,10,3,0,11,0,6,11,0,4,6,-1,-1,-1},
{12,4,11,8,4,6,11,0,2,9,2,10,9,-1,-1,-1},
{15,10,9,3,10,3,2,9,4,3,11,3,6,4,6,3},
{ 9,8,2,3,8,4,2,4,6,2,-1,-1,-1,-1,-1,-1},
{ 6,0,4,2,4,6,2,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{12,1,9,0,2,3,4,2,4,6,4,3,8,-1,-1,-1},
{ 9,1,9,4,1,4,2,2,4,6,-1,-1,-1,-1,-1,-1},
{12,8,1,3,8,6,1,8,4,6,6,10,1,-1,-1,-1},
{ 9,10,1,0,10,0,6,6,0,4,-1,-1,-1,-1,-1,-1},
{15,4,6,3,4,3,8,6,10,3,0,3,9,10,9,3},
{ 6,10,9,4,6,10,4,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,4,9,5,7,6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,0,8,3,4,9,5,11,7,6,-1,-1,-1,-1,-1,-1},
{ 9,5,0,1,5,4,0,7,6,11,-1,-1,-1,-1,-1,-1},
{12,11,7,6,8,3,4,3,5,4,3,1,5,-1,-1,-1},
{ 9,9,5,4,10,1,2,7,6,11,-1,-1,-1,-1,-1,-1},
{12,6,11,7,1,2,10,0,8,3,4,9,5,-1,-1,-1},
{12,7,6,11,5,4,10,4,2,10,4,0,2,-1,-1,-1},
{15,3,4,8,3,5,4,3,2,5,10,5,2,11,7,6},
{ 9,7,2,3,7,6,2,5,4,9,-1,-1,-1,-1,-1,-1},
{12,9,5,4,0,8,6,0,6,2,6,8,7,-1,-1,-1},
{12,3,6,2,3,7,6,1,5,0,5,4,0,-1,-1,-1},
{15,6,2,8,6,8,7,2,1,8,4,8,5,1,5,8},
{12,9,5,4,10,1,6,1,7,6,1,3,7,-1,-1,-1},
{15,1,6,10,1,7,6,1,0,7,8,7,0,9,5,4},
{15,4,0,10,4,10,5,0,3,10,6,10,7,3,7,10},
{12,7,6,10,7,10,8,5,4,10,4,8,10,-1,-1,-1},
{ 9,6,9,5,6,11,9,11,8,9,-1,-1,-1,-1,-1,-1},
{12,3,6,11,0,6,3,0,5,6,0,9,5,-1,-1,-1},
{12,0,11,8,0,5,11,0,1,5,5,6,11,-1,-1,-1},
{ 9,6,11,3,6,3,5,5,3,1,-1,-1,-1,-1,-1,-1},
{12,1,2,10,9,5,11,9,11,8,11,5,6,-1,-1,-1},
{15,0,11,3,0,6,11,0,9,6,5,6,9,1,2,10},
{15,11,8,5,11,5,6,8,0,5,10,5,2,0,2,5},
{12,6,11,3,6,3,5,2,10,3,10,5,3,-1,-1,-1},
{12,5,8,9,5,2,8,5,6,2,3,8,2,-1,-1,-1},
{ 9,9,5,6,9,6,0,0,6,2,-1,-1,-1,-1,-1,-1},
{15,1,5,8,1,8,0,5,6,8,3,8,2,6,2,8},
{ 6,1,5,6,2,1,6,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{15,1,3,6,1,6,10,3,8,6,5,6,9,8,9,6},
{12,10,1,0,10,0,6,9,5,0,5,6,0,-1,-1,-1},
{ 6,0,3,8,5,6,10,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,10,5,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,11,5,10,7,5,11,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,11,5,10,11,7,5,8,3,0,-1,-1,-1,-1,-1,-1},
{ 9,5,11,7,5,10,11,1,9,0,-1,-1,-1,-1,-1,-1},
{12,10,7,5,10,11,7,9,8,1,8,3,1,-1,-1,-1},
{ 9,11,1,2,11,7,1,7,5,1,-1,-1,-1,-1,-1,-1},
{12,0,8,3,1,2,7,1,7,5,7,2,11,-1,-1,-1},
{12,9,7,5,9,2,7,9,0,2,2,11,7,-1,-1,-1},
{15,7,5,2,7,2,11,5,9,2,3,2,8,9,8,2},
{ 9,2,5,10,2,3,5,3,7,5,-1,-1,-1,-1,-1,-1},
{12,8,2,0,8,5,2,8,7,5,10,2,5,-1,-1,-1},
{12,9,0,1,5,10,3,5,3,7,3,10,2,-1,-1,-1},
{15,9,8,2,9,2,1,8,7,2,10,2,5,7,5,2},
{ 6,1,3,5,3,7,5,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,0,8,7,0,7,1,1,7,5,-1,-1,-1,-1,-1,-1},
{ 9,9,0,3,9,3,5,5,3,7,-1,-1,-1,-1,-1,-1},
{ 6,9,8,7,5,9,7,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,5,8,4,5,10,8,10,11,8,-1,-1,-1,-1,-1,-1},
{12,5,0,4,5,11,0,5,10,11,11,3,0,-1,-1,-1},
{12,0,1,9,8,4,10,8,10,11,10,4,5,-1,-1,-1},
{15,10,11,4,10,4,5,11,3,4,9,4,1,3,1,4},
{12,2,5,1,2,8,5,2,11,8,4,5,8,-1,-1,-1},
{15,0,4,11,0,11,3,4,5,11,2,11,1,5,1,11},
{15,0,2,5,0,5,9,2,11,5,4,5,8,11,8,5},
{ 6,9,4,5,2,11,3,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{12,2,5,10,3,5,2,3,4,5,3,8,4,-1,-1,-1},
{ 9,5,10,2,5,2,4,4,2,0,-1,-1,-1,-1,-1,-1},
{15,3,10,2,3,5,10,3,8,5,4,5,8,0,1,9},
{12,5,10,2,5,2,4,1,9,2,9,4,2,-1,-1,-1},
{ 9,8,4,5,8,5,3,3,5,1,-1,-1,-1,-1,-1,-1},
{ 6,0,4,5,1,0,5,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{12,8,4,5,8,5,3,9,0,5,0,3,5,-1,-1,-1},
{ 3,9,4,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,4,11,7,4,9,11,9,10,11,-1,-1,-1,-1,-1,-1},
{12,0,8,3,4,9,7,9,11,7,9,10,11,-1,-1,-1},
{12,1,10,11,1,11,4,1,4,0,7,4,11,-1,-1,-1},
{15,3,1,4,3,4,8,1,10,4,7,4,11,10,11,4},
{12,4,11,7,9,11,4,9,2,11,9,1,2,-1,-1,-1},
{15,9,7,4,9,11,7,9,1,11,2,11,1,0,8,3},
{ 9,11,7,4,11,4,2,2,4,0,-1,-1,-1,-1,-1,-1},
{12,11,7,4,11,4,2,8,3,4,3,2,4,-1,-1,-1},
{12,2,9,10,2,7,9,2,3,7,7,4,9,-1,-1,-1},
{15,9,10,7,9,7,4,10,2,7,8,7,0,2,0,7},
{15,3,7,10,3,10,2,7,4,10,1,10,0,4,0,10},
{ 6,1,10,2,8,7,4,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,4,9,1,4,1,7,7,1,3,-1,-1,-1,-1,-1,-1},
{12,4,9,1,4,1,7,0,8,1,8,7,1,-1,-1,-1},
{ 6,4,0,3,7,4,3,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,4,8,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,9,10,8,10,11,8,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,3,0,9,3,9,11,11,9,10,-1,-1,-1,-1,-1,-1},
{ 9,0,1,10,0,10,8,8,10,11,-1,-1,-1,-1,-1,-1},
{ 6,3,1,10,11,3,10,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,1,2,11,1,11,9,9,11,8,-1,-1,-1,-1,-1,-1},
{12,3,0,9,3,9,11,1,2,9,2,11,9,-1,-1,-1},
{ 6,0,2,11,8,0,11,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,3,2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 9,2,3,8,2,8,10,10,8,9,-1,-1,-1,-1,-1,-1},
{ 6,9,10,2,0,9,2,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{12,2,3,8,2,8,10,0,1,8,1,10,8,-1,-1,-1},
{ 3,1,10,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 6,1,3,8,9,1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,0,9,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 3,0,3,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{ 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

//	for(int j=0; j<256; ++j){
//		printf("{");
//		int numTri=0;
//		for(int i=0; sTriTable[j][i] != -1; i+=3) ++numTri;
//
//		printf("%2d", numTri*3);
//		for(int i=0; i<15; ++i){
//			printf(",%d", sTriTable[j][i]);
//		}
//		printf("},\n");
//	}


Isosurface::NoVertexAction Isosurface::noVertexAction;

Isosurface::Isosurface(float lev, VertexAction& va)
:	mIsolevel(lev), mVertexAction(&va),
	mValidSurface(false), mComputeNormals(true), mNormalize(true), mInBox(false)
{
	cellLengths(1);
	fieldDims(0);
}

Isosurface::~Isosurface(){}

/*
Edge No:	integer identifier for local edge within cube
Edge ID:	unique integer for cube edge within entire domain of cubes
			format is 3*v + e where v is cube vertex and e is 0, 1, or 2 for x, y, or z direction edge
Vertex ID:	unique integer for cube position within entire domain of cubes

Cube-by-cube pass:

	1. Check eight values on corners of cube against isolevel
		a. Bitwise OR corner tests to obtain surface type (256 possible)

	2. Use surface type to look up what edges are intersected
		a. For each edge, compute point where surface intersects
		b. Store vertex in edge-to-vertex map

	3. Use surface type to obtain triangle indices (in terms of edge ID)
		a. Compute edge ID as combination of vertex ID and edge number


Compression pass:

	1. For each edge in edge-to-vertex map:
		a. Add edge vertex position to vertex buffer
		b. Assign index in vertex buffer to vertex.index

	2. For each triangle:
		a. Look up vertex position in edge-to-vertex map
		b. Append vertex.index to index buffer

	3. Generate per-vertex normals

*/

void Isosurface::addCell(const int * cellIdx3, const float * vals){
	const int &ix = cellIdx3[0];
	const int &iy = cellIdx3[1];
	const int &iz = cellIdx3[2];

	// Get isosurface cell index depending on field values at corners of cell
	int idx = 0;
	if(vals[0] < level()) idx |=   1;
	if(vals[2] < level()) idx |=   2;
	if(vals[3] < level()) idx |=   4;
	if(vals[1] < level()) idx |=   8;
	if(vals[4] < level()) idx |=  16;
	if(vals[6] < level()) idx |=  32;
	if(vals[7] < level()) idx |=  64;
	if(vals[5] < level()) idx |= 128;

	// Create a triangulation of the isosurface in this cell
	const int edgeCode = sEdgeTable[idx];
	if(edgeCode){

		int cID = cellID(ix,iy,iz);

		// Compute interpolated vertices on edges of box
		//		(int EdgeID, PointID vertex) pair is created in mEdgeToVertex
		//		EdgeID is a unique integer ID of box edge
		if(edgeCode &    1) addEdgeVertex(ix,iy,iz,cID, 0, vals);
		if(edgeCode &    2) addEdgeVertex(ix,iy,iz,cID, 1, vals);
		if(edgeCode &    4) addEdgeVertex(ix,iy,iz,cID, 2, vals);
		if(edgeCode &    8) addEdgeVertex(ix,iy,iz,cID, 3, vals);
		if(edgeCode &   16) addEdgeVertex(ix,iy,iz,cID, 4, vals);
		if(edgeCode &   32) addEdgeVertex(ix,iy,iz,cID, 5, vals);
		if(edgeCode &   64) addEdgeVertex(ix,iy,iz,cID, 6, vals);
		if(edgeCode &  128) addEdgeVertex(ix,iy,iz,cID, 7, vals);
		if(edgeCode &  256) addEdgeVertex(ix,iy,iz,cID, 8, vals);
		if(edgeCode &  512) addEdgeVertex(ix,iy,iz,cID, 9, vals);
		if(edgeCode & 1024) addEdgeVertex(ix,iy,iz,cID,10, vals);
		if(edgeCode & 2048) addEdgeVertex(ix,iy,iz,cID,11, vals);

		// Add up to 5 triangles (15 vertices) representing surface through cell
		// Each triangle consists of 3 vertex indices stored in mEdgeToVertex
		for(int i=1; i <= sTriTable[idx][0]; i+=3){
			// Add 3 edge indices of triangle
			index(edgeID(cID, sTriTable[idx][i  ]));
			index(edgeID(cID, sTriTable[idx][i+1]));
			index(edgeID(cID, sTriTable[idx][i+2]));
		}
	}
}


void Isosurface::addEdgeVertex(int ix, int iy, int iz, int cellID, int edgeNo, const float * vals){

	int eIdx = edgeID(cellID, edgeNo);

	if(inBox()){
		if(mEdgeToVertexArray[eIdx] < 0){
			EdgeVertex ev = calcIntersection(ix,iy,iz, edgeNo, vals);

			ev.pos[0] = ix;
			ev.pos[1] = iy;
			ev.pos[2] = iz;

			int vIdx = Mesh::vertices().size();
			mEdgeToVertexArray[eIdx] = vIdx;

			Mesh::vertex(ev.x, ev.y, ev.z);

			(*mVertexAction)(ev, *this);
		}
	}
	else{
		EdgeToVertex::iterator it = mEdgeToVertex.find(eIdx);

		// If this edge vertex has not been computed yet, then compute it
		if(mEdgeToVertex.end() == it){
			EdgeVertex ev = calcIntersection(ix,iy,iz, edgeNo, vals);

			ev.pos[0] = ix;
			ev.pos[1] = iy;
			ev.pos[2] = iz;
	//		ev.idx = cellID/3;

			int vIdx = Mesh::vertices().size();
			mEdgeToVertex[eIdx] = vIdx;

			Mesh::vertex(ev.x, ev.y, ev.z);

			(*mVertexAction)(ev, *this);

	//		if(cols) Mesh::color(cols[ev.i0] + ev.mu * (cols[ev.i1] - cols[ev.i0]));

	//		if(1) Mesh::color(HSV((vIdx % 100)/100., 1,1));
		}
	}
};


Isosurface::EdgeVertex
Isosurface::calcIntersection(int ix, int iy, int iz, int edgeNo, const float * vals) const{

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
	static const char edgeCorners[12][2] = {
		{0,2}, {2,3}, {3,1}, {1,0},
		{4,6}, {6,7}, {7,5}, {5,4},
		{0,4}, {2,6}, {3,7}, {1,5}
	};

	const char& c0 = edgeCorners[edgeNo][0];
	const char& c1 = edgeCorners[edgeNo][1];
	const float& val1 = vals[c0];
	const float& val2 = vals[c1];
	const int * e1 = cubeVertices[c0];
	const float * ed = edgeDirections[edgeNo];

	// Interpolate between two grid points to produce the point at which
	// the isosurface intersects an edge.

	// 'mu' is the fraction along the edge where the vertex lies
	float mu = float((level() - val1)/(val2 - val1));

	EdgeVertex r;
	r.x = (ix + e1[0] + mu*ed[0]) * mL[0];
	r.y = (iy + e1[1] + mu*ed[1]) * mL[1];
	r.z = (iz + e1[2] + mu*ed[2]) * mL[2];

//	r.corners[0] = i0;
//	r.corners[1] = i1;
	for(int i=0; i<3; ++i){
		r.corners[0][i] = cubeVertices[c0][i];
		r.corners[1][i] = cubeVertices[c1][i];
	}
	r.mu=mu;

	return r;
}


void Isosurface::begin(){
	mValidSurface = false;
	reset();
}


void Isosurface::end(){
	compressTriangles();
	triangles(); // primitive must be set for proper normal generation
	if(mComputeNormals) generateNormals(mNormalize);
	mValidSurface = true;
}


// Compress vertices and triangles so that they can be accessed more efficiently
void Isosurface::compressTriangles(){

	// Convert edge indices into vertex buffer indices

	if(inBox()){
		for(int i=0; i<indices().size(); ++i){
			int ei = indices()[i];
			int vi = mEdgeToVertexArray[ei];
			indices()[i] = vi;
		}
		mEdgeToVertexArray.assign(mEdgeToVertexArray.size(), -1);
	}

	// Lookup vertex buffer indices in map
	// This is slower, but uses less memory
	else{
		for(int i=0; i<indices().size(); ++i){
			int ei = indices()[i];
			int vi = mEdgeToVertex[ei];
			indices()[i] = vi;
		}
	}

	mEdgeToVertex.clear();
	mEdgeTriangles.reset();
}


Isosurface& Isosurface::cellLengths(double dx, double dy, double dz){
	mL[0]=dx; mL[1]=dy; mL[2]=dz;
	return *this;
}


Isosurface& Isosurface::fieldDims(int nx, int ny, int nz){
	mNF[0] = nx;
	mNF[1] = ny;
	mNF[2] = nz;

	// offsets for edges going in positive directions at each corner
	static const int ex = 0;
	static const int ey = 1;
	static const int ez = 2;

	mEdgeIDOffsets[ 3] = ex;
	mEdgeIDOffsets[ 0] = ey;
	mEdgeIDOffsets[ 8] = ez;

	mEdgeIDOffsets[ 2] = ey + 3;
	mEdgeIDOffsets[11] = ez + 3;

	mEdgeIDOffsets[ 1] = ex + 3*mNF[0];
	mEdgeIDOffsets[ 9] = ez + 3*mNF[0];

	mEdgeIDOffsets[ 7] = ex + 3*mNF[0]*mNF[1];
	mEdgeIDOffsets[ 4] = ey + 3*mNF[0]*mNF[1];

	mEdgeIDOffsets[10] = ez + 3*(1 + mNF[0]                );
	mEdgeIDOffsets[ 5] = ex + 3*(    mNF[0] + mNF[0]*mNF[1]);
	mEdgeIDOffsets[ 6] = ey + 3*(1          + mNF[0]*mNF[1]);

	return *this;
}


Isosurface& Isosurface::inBox(bool v){
	mInBox=v;
	if(inBox()){
		unsigned numEdges = 3 * (mNF[0]+1) * (mNF[1]+1) * (mNF[2]+1);
		if(numEdges != mEdgeToVertexArray.size()){
			mEdgeToVertexArray.resize(numEdges);
			mEdgeToVertexArray.assign(numEdges, -1);
		}
	}
	return *this;
}


bool Isosurface::volumeLengths(double& volLengthX, double& volLengthY, double& volLengthZ) const {
	if(validSurface()){
		volLengthX = mL[0]*(mNF[0]-1);
		volLengthY = mL[1]*(mNF[1]-1);
		volLengthZ = mL[2]*(mNF[2]-1);
		return true;
	}
	return false;
}

void Isosurface::generate(const Voxels& voxels, float glUnitLength) {
	generate((float*)voxels.data.ptr, voxels.dim(0), voxels.dim(1), voxels.dim(2),
		voxels.getVoxWidth(0)/glUnitLength, voxels.getVoxWidth(1)/glUnitLength, voxels.getVoxWidth(2)/glUnitLength);
}

} // al::

/*

Edge index:
22 - 31		cube z coord		[0, 1023]
12 - 21		cube y coord		[0, 1023]
02 - 11		cube x coord		[0, 1023]
00 - 01		cube edge number	[0,    3] -> [x, y, z, *]

edgeIdx = edgeNo | (ix << 2) | (iy << 12) | (iz << 22);

edgeNo = edgeIdx & 3;
ix = (edgeIdx >>  2) & 1023;
iy = (edgeIdx >> 12) & 1023;
iz = (edgeIdx >> 22) & 1023;

int edgeID(int vertID, int edgeNo) const{ return vertID + mEdgeIDOffsets[edgeNo]; }

int vertexID(int ix, int iy, int iz) const{ return 3*(ix + mNF[0] * (iy + mNF[1] * iz)); }


TODO:
- optimize for multiple isolevels

Biggest hit is lookup into the edge-to-vertex map.

Speed improvements:

	1. Use an array rather than map for storing edge vertices.
		Q. How will edges be assigned to array indices?
		A. Edge IDs must be range reduced. We must know the position
			of the isosurface before doing marching cubes. Field indices must
			lie in range (0,0,0) to (N1, N2, N3).

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
