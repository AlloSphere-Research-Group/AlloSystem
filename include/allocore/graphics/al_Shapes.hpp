#ifndef INCLUDE_AL_GRAPHICS_SHAPES_HPP
#define INCLUDE_AL_GRAPHICS_SHAPES_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


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

/// @param[in,out]	m		mesh to add vertices and indices to
/// @param[in]		radius	radius of sphere
/// @param[in]		slices	number of slices around z axis
/// @param[in]		stacks	number of stacks on xy plane
int addSphere(Mesh& m, double radius=1, int slices=8, int stacks=8);


/// Add wireframe box as line vertices and indices
int addWireBox(Mesh& m, float width, float height, float depth);
inline int addWireBox(Mesh& m, float size=1){ return addWireBox(m,size,size,size); }


/// Add a tessellated rectangular surface; render with triangle strip
int addSurface(Mesh& m, int dimX, int dimY, float width=2, float height=2);


} // al::

#endif
