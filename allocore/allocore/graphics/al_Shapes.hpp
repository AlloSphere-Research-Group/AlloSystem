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

#include <cmath>
#include <functional>

namespace al{

class Mesh;

/// @addtogroup allocore
/// @{

/// Add tetrahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (4)
int addTetrahedron(Mesh& m, float radius=1);

/// Add rectangular cuboid as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		rx		Center to face along x
/// @param[in]		ry		Center to face along y
/// @param[in]		rz		Center to face along z
/// \returns number of vertices added (8)
int addCuboid(Mesh& m, float rx, float ry, float rz);
inline int addCuboid(Mesh& m, float radius=1){ return addCuboid(m,radius,radius,radius); }
inline int addCuboid(Mesh& m, const Vec3f& radii){ return addCuboid(m,radii[0],radii[1],radii[2]); }

/// Add cube as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of cube from center to faces;
///							sqrt(1/3) gives cube inscribed in unit sphere
/// \returns number of vertices added (8)
inline int addCube(Mesh& m, float radius=M_SQRT_1_3){ return addCuboid(m,radius); }

/// Add octahedron as triangle vertices and indices

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (6)
int addOctahedron(Mesh& m, float radius=1);

/// Add dodecahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (20)
int addDodecahedron(Mesh& m, float radius=1);

/// Add icosahedron as indexed triangles

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		radius	Radius of enclosing sphere
/// \returns number of vertices added (12)
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
int addSphere(Mesh& m, double radius=1, int slices=16, int stacks=16);
int addSphereWithTexcoords(Mesh& m, double radius=1, int bands=16 );


/// Add wireframe box as indexed lines

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		pMin	Corner with minimum coordinate values
/// @param[in]		pMax	Corner with maximum coordinate values
/// \returns number of vertices added
int addWireBox(Mesh& m, const Vec3f& pMin, const Vec3f& pMax);

/// Add wireframe box as indexed lines

/// The box is centered at (0,0,0).
///
/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		width	Total width (along x)
/// @param[in]		height	Total height (along y)
/// @param[in]		depth	Total depth (along z)
/// \returns number of vertices added
int addWireBox(Mesh& m, float width, float height, float depth);
inline int addWireBox(Mesh& m, float size=1){ return addWireBox(m,size,size,size); }


/// Add grid made out of parallel lines

/// @param[in,out]	m		Mesh to add vertices and indices to
/// @param[in]		n1		Number of cells along first dimension
/// @param[in]		n2		Number of cells along second dimension
/// @param[in]		radii	Radii of grid
/// @param[in]		center	Center of grid
/// \returns number of vertices added
template <int Dim1=0, int Dim2=1>
int addWireGrid(Mesh& m, int n1, int n2, Vec2f radii = 1, Vec2f center = 0);


/// Add a cone/pyramid as indexed triangles

/// Note that the base lies on the xy plane, thus the shape is not "centered"
/// on the z axis.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] radius		Radius of base (on xy plane)
/// @param[in] apex			Position of apex
/// @param[in] slices		Number of points going around base
/// @param[in] stacks		Number of rings from apex to base
/// @param[in] cycles		Number of cycles to go around base
///							(should be relatively prime to slices)
/// \returns number of vertices added
int addCone(
	Mesh& m, float radius=1, const Vec3f& apex=Vec3f(0,0,2),
	unsigned slices=16, unsigned stacks=1,
	unsigned cycles=1
);


/// Add a disc/regular polygon as indexed triangles

/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] radius		Radius of disc (on xy plane)
/// @param[in] slices		Number of points going around base
/// @param[in] stacks		Number of rings from origin to outer radius
/// \returns number of vertices added
int addDisc(Mesh& m, float radius=1, unsigned slices=16, unsigned stacks=1);


/// Add a rectangle as indexed triangles

/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] width		Total width (along x)
/// @param[in] height		Total height (along y)
/// @param[in] x			Position of center along x
/// @param[in] y			Position of center along y
///
/// Vertex order is bottom-left, bottom-right, top-left, top-right.
///
/// \returns number of vertices added
int addRect(Mesh& m, float width=2, float height=2, float x=0, float y=0);


/// Add a quadrilateral as indexed triangles

/// Input points should be co-planar and in counter-clockwise winding order.
///
/// \returns number of vertices added
template <class Vec>
int addQuad(Mesh& m, const Vec& a, const Vec& b, const Vec& c, const Vec& d);

int addQuad(
	Mesh& m,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3,
	float x4, float y4, float z4
);


/// Add a prism as indexed triangles

/// A prism is formed from the vertices of two parallel regular polygons.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] btmRadius	Radius of bottom polygon (on xy plane)
/// @param[in] topRadius	Radius of top polygon (on xy plane)
/// @param[in] height		Distance between planes (along z axis)
/// @param[in] slices		Number of polygon vertices
/// @param[in] twist		Rotation factor between polygons;
///							a value of 0.5 produces an antiprism
/// @param[in] caps			Whether to generate end caps
/// \returns number of vertices added
int addPrism(
	Mesh& m, float btmRadius=1, float topRadius=1, float height=2,
	unsigned slices=16,
	float twist=0,
	bool caps=true
);


/// Add an annulus ("little ring") as indexed triangles

/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] inRadius		Radius of inner circle (on xy plane)
/// @param[in] outRadius	Radius of outer circle (on xy plane)
/// @param[in] slices		Number of polygon vertices
/// @param[in] twist		Rotation factor between polygons
/// \returns number of vertices added
int addAnnulus(
	Mesh& m, float inRadius=0.5, float outRadius=1,
	unsigned slices=16,
	float twist=0
);


/// Add a cylinder as indexed triangles

/// To create a cylinder with different radii for the top and bottom, /see
/// addPrism.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] radius		Radius (on xy plane)
/// @param[in] height		Height (along z axis)
/// @param[in] slices		Number of polygon vertices
/// @param[in] twist		Rotation factor between polygons
/// @param[in] caps			Whether to generate end caps
/// \returns number of vertices added
int addCylinder(
	Mesh& m, float radius=1, float height=2,
	unsigned slices=16,
	float twist=0,
	bool caps=true
);


/// Add a tessellated rectangular surface as an indexed triangle strip

/// This creates a flat, regularly-tesselated surface lying on the xy plane.
/// The surface normal points along +z with counter-clockwise triangle winding.
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
int addTorus(
	Mesh& m, double minRadius=0.3, double majRadius=0.7,
	int Nmin=16, int Nmaj=16, double minPhase=0
);


/// Add cubic voxels (as indexed triangles and normals)

/// Inside of voxels have positive, non-zero values while outside have the
/// value of zero. Generated normals point from inside to outside.
/// The span of the voxel geometry is (0,0,0) to (Nx,Ny,Nz)*cellSize.
///
/// @param[in,out] m		Mesh to add vertices and indices to
/// @param[in] getVoxel		Function that returns voxel value at index
/// @param[in] Nx			Voxel samples along x
/// @param[in] Ny			Voxel samples along y
/// @param[in] Nz			Voxel samples along z
/// @param[in] cellSize		Length of each rendered voxel cell
/// @param[in] onFace		Called when quad face (4 verts) added to mesh;
///							argument is index of first vertex of quad
/// \returns number of vertices added
int addVoxels(
	Mesh& m,
	const std::function<float(int x, int y, int z)>& getVoxel,
	int Nx, int Ny, int Nz, float cellSize = 0.1,
	const std::function<void(int vertex)>& onFace = [](int){}
);


/// Fill array with ellipse (using fast recursion method)
template <class Vec2>
void ellipse(Vec2 * dst, int len, float radx=1, float rady=1);


/// Fill array with circle (using fast recursion method)
template <class Vec2>
void circle(Vec2 * dst, int len, float rad=1){ ellipse(dst,len, rad,rad); }

/// @} // end allocore group



// Implementation only below
template <class Vec>
int addQuad(Mesh& m, const Vec& a, const Vec& b, const Vec& c, const Vec& d){
	return addQuad(m, a[0],a[1],a[2], b[0],b[1],b[2], c[0],c[1],c[2], d[0],d[1],d[2]);
}

template <int Dim1, int Dim2>
int addWireGrid(Mesh& m, int n1, int n2, Vec2f radii, Vec2f center){
	m.lines();
	
	auto mn = center - radii;
	auto mx = center + radii;

	for(int i=0; i<n1+1; ++i){
		float x = (float(i)/n1*2.-1.)*radii[0] + center[0];
		m.vertex(Vec3f().template set<Dim1>(x).template set<Dim2>(mn[1]));
		m.vertex(Vec3f().template set<Dim1>(x).template set<Dim2>(mx[1]));
	}

	for(int i=0; i<n2+1; ++i){
		float y = (float(i)/n2*2.-1.)*radii[1] + center[1];
		m.vertex(Vec3f().template set<Dim2>(y).template set<Dim1>(mn[0]));
		m.vertex(Vec3f().template set<Dim2>(y).template set<Dim1>(mx[0]));
	}

	return (n1+1)*2 + (n2+1)*2;
}

template <class Vec2>
void ellipse(Vec2 * dst, int len, float radx, float rady){
	struct RSin{
		float mul,val,val2;
		RSin(float f=1, float a=1, float p=0){
			static const float twoPi = 6.283185307179586476925286766559;
			f *= twoPi, p *= twoPi;
			mul  = 2. * std::cos(f);
			val2 = std::sin(p - f * 2.)*a;
			val  = std::sin(p - f     )*a;
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
