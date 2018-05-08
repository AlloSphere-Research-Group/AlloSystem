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

/// Add tetrahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (4)
///
/// @ingroup allocore
int addTetrahedron(Mesh& m, float radius=1);

/// Add cube as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (8)
///
/// @ingroup allocore
int addCube(Mesh& m, bool withNormalsAndTexcoords = false, float radius=M_SQRT_1_3);

/// Add octahedron as triangle vertices and indices

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (6)
///
/// @ingroup allocore
int addOctahedron(Mesh& m, float radius=1);

/// Add dodecahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (20)
///
/// @ingroup allocore
int addDodecahedron(Mesh& m, float radius=1);

/// Add icosahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (12)
///
/// @ingroup allocore
int addIcosahedron(Mesh& m, float radius=1);


/// Add sphere produced from subdivided icosahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of sphere
/// @param[in]		divisions Number of recursive subdivisions
/// \returns number of vertices added
int addIcosphere(Mesh& m, double radius=1, int divisions=2);


/// Add sphere as indexed triangles

/// Vertices go stack-by-stack, then slice-by-slice. The stacks start at the
/// north pole (0,0,radius) and end at the south pole (0,0,-radius).
/// The slices start on the x axis and go counter-clockwise on the xy plane.
///
/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of sphere
/// @param[in]		slices	Number of slices around z axis
/// @param[in]		stacks	Number of stacks on xy plane
/// \returns number of vertices added
///
/// @ingroup allocore
int addSphere(Mesh& m, double radius=1, int slices=16, int stacks=16);
int addSphereWithTexcoords(Mesh& m, double radius=1, int bands=16 );


/// Add wireframe box as indexed lines

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		width	Total width (along x)
/// @param[in]		height	Total height (along y)
/// @param[in]		depth	Total depth (along z)
/// \returns number of vertices added
///
/// @ingroup allocore
int addWireBox(Mesh& m, float width, float height, float depth);
inline int addWireBox(Mesh& m, float size=1){ return addWireBox(m,size,size,size); }


/// Add a cone/pyramid as indexed triangles

/// Note that the base lies on the xy plane, thus the shape is not "centered"
/// on the z axis.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] radius		Radius of base (on xy plane)
/// @param[in] apex			Position of apex
/// @param[in] slices		Number of points going around base
/// @param[in] cycles		Number of cycles to go around base
///							(should be relatively prime to slices)
/// \returns number of vertices added
///
/// @ingroup allocore
int addCone(
	Mesh& m, float radius=1, const Vec3f& apex=Vec3f(0,0,2),
	unsigned slices=16,
	unsigned cycles=1
);


/// Add a disc/regular polygon as indexed triangles

/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] radius		Radius of disc (on xy plane)
/// @param[in] slices		Number of points going around base
/// \returns number of vertices added
///
/// @ingroup allocore
int addDisc(Mesh& m, float radius=1, unsigned slices=16);


/// Add a rectangle as indexed triangles

/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] width		Total width (along x)
/// @param[in] height		Total height (along y)
/// @param[in] x			Position of center along x
/// @param[in] y			Position of center along y
/// \returns number of vertices added
///
/// @ingroup allocore
int addRect(Mesh& m, float width=2, float height=2, float x=0, float y=0);


/// Add a prism as an indexed triangle strip

/// A prism is formed from a triangle strip between two parallel regular
/// polygons.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] btmRadius	Radius of bottom polygon (on xy plane)
/// @param[in] topRadius	Radius of top polygon (on xy plane)
/// @param[in] height		Distance between planes (along z axis)
/// @param[in] slices		Number of polygon vertices
/// @param[in] twist		Rotation factor between polygons;
///							a value of 0.5 produces an antiprism
/// \returns number of vertices added
///
/// @ingroup allocore
int addPrism(
	Mesh& m, float btmRadius=1, float topRadius=1, float height=2,
	unsigned slices=16,
	float twist=0
);


/// Add an annulus ("little ring") as an indexed triangle strip

/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] inRadius		Radius of inner circle (on xy plane)
/// @param[in] outRadius	Radius of outer circle (on xy plane)
/// @param[in] slices		Number of polygon vertices
/// @param[in] twist		Rotation factor between polygons
/// \returns number of vertices added
///
/// @ingroup allocore
int addAnnulus(
	Mesh& m, float inRadius=0.5, float outRadius=1,
	unsigned slices=16,
	float twist=0
);


/// Add an open cylinder as an indexed triangle strip

/// To create a cylinder with different radii for the top and bottom, /see
/// addPrism.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] radius		Radius (on xy plane)
/// @param[in] height		Height (along z axis)
/// @param[in] slices		Number of polygon vertices
/// @param[in] twist		Rotation factor between polygons
/// \returns number of vertices added
///
/// @ingroup allocore
int addCylinder(
	Mesh& m, float radius=1, float height=2,
	unsigned slices=16,
	float twist=0
);


/// Add a tessellated rectangular surface as an indexed triangle strip

/// This creates a flat, regularly-tesselated surface lying on the xy plane.
/// This shape can be used as a starting point for more complex meshes such as 
/// height maps/terrains and texture-mapped spheres and torii.
///
/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		Nx		Number of vertices along x
/// @param[in]		Ny		Number of vertices along y
/// @param[in]		width	Total width (along x)
/// @param[in]		height	Total height (along y)
/// @param[in]		x		Position of center along x
/// @param[in]		y		Position of center along y
/// \returns number of vertices added
///
/// @ingroup allocore
int addSurface(
	Mesh& m, int Nx, int Ny,
	double width=2, double height=2, double x=0, double y=0
);


/// Add a tessellated rectangular surface with connected edges

/// This adds a rectangular surface whose edges are connected. The resulting
/// surface is suitable for warping into cylindrical or toroidal shapes.
/// Given D as the width or height, the interval of position values along a
/// particular dimension is closed-opened, [-D/2, D/2), if that dimension loops
/// or closed, [-D/2, D/2], if that dimension does not loop.
/// The drawing primitive is assumed to be a triangle strip.
///
/// @param[in,out] m	Mesh to add vertices and indices to
/// @param[in] Nx		Number of vertices along x
/// @param[in] Ny		Number of vertices along y
/// @param[in] loopMode	1: connect edges perpendicular to x (cylindrical),
///						2: connect edges perpendicular to x and y (toroidal)
/// @param[in] width	Total width (along x)
/// @param[in] height	Total height (along y)
/// @param[in] x		Position of center along x
/// @param[in] y		Position of center along y
/// \returns number of vertices added
///
/// @ingroup allocore
int addSurfaceLoop(
	Mesh& m, int Nx, int Ny, int loopMode,
	double width=2, double height=2, double x=0, double y=0
);


/// Add a torus as an indexed triangle strip

/// If you need a texture-mapped torus, /see addSurface.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] minRadius	Radius of minor ring
/// @param[in] majRadius	Radius of major ring
/// @param[in] Nmin			Number of vertices around minor ring
/// @param[in] Nmaj			Number of vertices around major ring
/// @param[in] minPhase		Starting phase along minor ring, in [0,1]
/// \returns number of vertices added
///
/// @ingroup allocore
int addTorus(
	Mesh& m, double minRadius=0.3, double majRadius=0.7,
	int Nmin=16, int Nmaj=16, double minPhase=0
);


/// Fill array with ellipse (using fast recursion method)
template <class Vec2>
void ellipse(Vec2 * dst, int len, float radx=1, float rady=1);

/// Fill array with circle (using fast recursion method)
template <class Vec2>
void circle(Vec2 * dst, int len, float rad=1){ ellipse(dst,len, rad,rad); }


// Implementation only below

template <class Vec2>
void ellipse(Vec2 * dst, int len, float radx, float rady){
	struct RSin{
		float mul,val,val2;
		RSin(float f=1, float a=1, float p=0){
			f *= M_2PI, p *= M_2PI;
			mul  = 2. * cos(f);
			val2 = sin(p - f * 2.)*a;
			val  = sin(p - f     )*a;
		}
		float operator()(){
			auto v0 = mul * val - val2;
			val2 = val;
			return val = v0;
		}
	};
	RSin x(1./len, radx, 0.25), y(1./len, rady);
	for(int i=0; i<len; ++i){
		dst[i][0] = x();
		dst[i][1] = y();
	}
}

} // al::

#endif
