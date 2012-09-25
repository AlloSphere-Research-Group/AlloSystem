#ifndef INCLUDE_AL_GRAPHICS_SHAPES_HPP
#define INCLUDE_AL_GRAPHICS_SHAPES_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, 
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright 
		notice, this list of conditions and the following disclaimer in the 
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its 
		contributors may be used to endorse or promote products derived from 
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Factory methods for standard geometries

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Mesh.hpp"

namespace al{

// Platonic solids

/// Add tetrahedron as triangle vertices and indices
int addTetrahedron(Mesh& m);

/// Add cube as triangle vertices and indices
int addCube(Mesh& m, bool withNormalsAndTexcoords = false, float radius=M_SQRT_1_3);

/// Add octahedron as triangle vertices and indices
int addOctahedron(Mesh& m);

/// Add dodecahedron as triangle vertices and indices
int addDodecahedron(Mesh& m);

/// Add icosahedron as triangle vertices and indices
int addIcosahedron(Mesh& m);


/// Add sphere as triangle vertices and indices

/// Vertices go stack-by-stack, then slice-by-slice. The stacks start at the
/// north pole (0,0,radius) and end at the south pole (0,0,-radius). 
/// The slices start on the x axis and go counter-clockwise on the xy plane.
///
/// @param[in,out]	m		mesh to add vertices and indices to
/// @param[in]		radius	radius of sphere
/// @param[in]		slices	number of slices around z axis
/// @param[in]		stacks	number of stacks on xy plane
int addSphere(Mesh& m, double radius=1, int slices=16, int stacks=16);


/// Add wireframe box as line vertices and indices
int addWireBox(Mesh& m, float width, float height, float depth);
inline int addWireBox(Mesh& m, float size=1){ return addWireBox(m,size,size,size); }


/// Add a tessellated rectangular surface; render with triangle strip
int addSurface(Mesh& m, int dimX, int dimY, float width=2, float height=2);


} // al::

#endif
