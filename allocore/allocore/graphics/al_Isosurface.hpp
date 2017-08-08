#ifndef INCLUDE_AL_ISOSURFACE_HPP
#define INCLUDE_AL_ISOSURFACE_HPP

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

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com

	This file incorporates work covered by the following copyright(s) and
	permission notice(s):

		Author: Raghavendra Chandrashekara
		Email: rc99@doc.ic.ac.uk, rchandrashekara@hotmail.com
		Last Modified: 5/8/2000

		This work incorporates work covered by the following copyright and
		permission notice:

			Marching Cubes Example Program
			by Cory Bloyd (corysama@yahoo.com)

			A simple, portable and complete implementation of the Marching Cubes
			and Marching Tetrahedrons algorithms in a single source file.
			There are many ways that this code could be made faster, but the
			intent is for the code to be easy to understand.

			For a description of the algorithm go to
			http://astronomy.swin.edu.au/pbourke/modelling/polygonise/

			This code is public domain.
*/

#include <map>
#include <unordered_map>
#include <vector>
#include "allocore/types/al_Buffer.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/types/al_Voxels.hpp"

namespace al{


/// Isosurface generated using marching cubes
/// @ingroup allocore
class Isosurface : public Mesh {
public:

	struct EdgeVertex {
		Vec3i pos;			// cell coordinates
		Vec3i corners[2];	// edge corners as offsets from cell coordinates
		float x, y, z;		// vertex position
		float mu;			// fraction along edge of vertex

		/// Returns position of edge vertices
		Vec3i edgePos(int i) const { return pos + corners[i]; }
	};

	struct EdgeTriangle {
		int edgeIDs[3];
	};

	struct VertexAction {
		virtual ~VertexAction(){}
		virtual void operator()(const Isosurface::EdgeVertex& v, Isosurface& s) = 0;
	};

	struct NoVertexAction : public VertexAction{
		virtual void operator()(const EdgeVertex& v, Isosurface& s){}
	};

	static NoVertexAction noVertexAction;


	/// @param[in] level	value to construct surface on
	/// @param[in] action	user defined functor called upon adding a new edge vertex
	Isosurface(float level=0, VertexAction& action = noVertexAction);

	virtual ~Isosurface();

	/// Get a field dimension
	int fieldDim(int i) const { return mNF[i]; }

	/// Get isolevel
	float level() const { return mIsolevel; }

	/// Returns true if a valid surface has been generated
	bool validSurface() const { return mValidSurface; }

	/// Gets the length, width, and height of the isosurface volume

	/// \returns true upon success and false if the surface is not valid
	///
	bool volumeLengths(double& volLengthX, double& volLengthY, double& volLengthZ) const;

	// Get unique identifier of 3d position indices
	int posID(int ix, int iy, int iz) const { return ix + mNF[0] * (iy + mNF[1] * iz); }

	template <class VEC3I>
	int posID(const VEC3I& i3) const { return posID(i3[0], i3[1], i3[2]); }

	// Get unique identifier of cell
	int cellID(int ix, int iy, int iz) const{ return 3*posID(ix,iy,iz); }

	// Get unique identifier of edge
	int edgeID(int cellID, int edgeNo) const{ return cellID + mEdgeIDOffsets[edgeNo]; }



	/// Set individual dimensions of the scalar field
	Isosurface& fieldDims(int nx, int ny, int nz);

	/// Set all dimensions of the scalar field
	Isosurface& fieldDims(int n){ return fieldDims(n,n,n); }

	/// Set individual lengths of cell
	Isosurface& cellLengths(double dx, double dy, double dz);

	/// Set all lengths of cell
	Isosurface& cellLengths(double v){ return cellLengths(v,v,v); }

	/// Set isolevel
	Isosurface& level(float v){ mIsolevel=v; return *this; }

	/// Set whether to compute normals
	Isosurface& normals(bool v){ mComputeNormals=v; return *this; }

	/// Set whether to normalize normals (if being computed)
	Isosurface& normalize(bool v){ mNormalize=v; return *this; }


	/// Begin cell-at-a-time mode
	void begin();

	/// End cell-at-a-time mode
	void end();


	/// Add a cell from a scalar field

	/// This should be called in between calls to begin() and end().
	///
	template <class T>
	void addCell(
		int ix, int iy, int iz,
		const T& xyz, const T& Xyz,
		const T& xYz, const T& XYz,
		const T& xyZ, const T& XyZ,
		const T& xYZ, const T& XYZ
	){
		int inds[3] = { ix, iy, iz };
		const float vals[8] = { xyz, Xyz, xYz, XYz, xyZ, XyZ, xYZ, XYZ };
		addCell(inds, vals);
	}


	/// Add a cell from a scalar field
	void addCell(const int * indices3, const float * values8);


	/// Generate isosurface from scalar field
	template <class T>
	void generate(const T * scalarField);


	/// Generate isosurface from scalar field

	/// The total number of elements in the field is expected to be nX*nY*nZ.
	/// The field elements are located at the corners of the cuboidal cells used
	/// to generate the surface. Thus, the surface is evaluated on a total of
	/// (nX-1)*(nY-1)*(nZ-1) cells.
	template <class T>
	void generate(const T * scalarField, int nX, int nY, int nZ, float cellLengthX, float cellLengthY, float cellLengthZ){
		fieldDims(nX, nY, nZ);
		cellLengths(cellLengthX, cellLengthY, cellLengthZ);
		generate(scalarField);
	}

	template <class T>
	void generate(const T * scalarField, int n, float cellLength){
		generate(scalarField, n,n,n, cellLength,cellLength,cellLength);
	}

	// support for building isosurface from al::Voxels class
	void generate(const Voxels& voxels, float glUnitLength) {
		generate((float*)voxels.data.ptr, voxels.dim(0), voxels.dim(1), voxels.dim(2),
			voxels.getVoxWidth(0)/glUnitLength, voxels.getVoxWidth(1)/glUnitLength, voxels.getVoxWidth(2)/glUnitLength);
	}

	void vertexAction(VertexAction& a){ mVertexAction = &a; }

	const bool inBox() const { return mInBox; }

	/// Set whether isosurface is assumed to fit snugly within a box

	/// Setting this to true will speed up extraction when the isosurface
	/// is mostly dense and box shaped, i.e., a rectangular cuboid. If the
	/// isosurface is to be computed on sparse or irregularly shaped grid, then
	/// it is recommended to set this to false to save memory.
	Isosurface& inBox(bool v);

protected:

	struct IsosurfaceHashInt{
		size_t operator()(int v) const { return v; }
	//	size_t operator()(int v) const { return v*2654435761UL; }
	};

	typedef std::unordered_map<int, int, IsosurfaceHashInt> EdgeToVertex;

	EdgeToVertex mEdgeToVertex;					// map from edge ID to vertex
	al::Buffer<EdgeTriangle> mEdgeTriangles;	// surface triangles in terms of edge IDs

	std::vector<int> mEdgeToVertexArray;
//	al::Buffer<int> mTempEdges;

	double mL[3];				// cell length in x, y, and z directions
	int mNF[3];					// number of field points in x, y, and z directions
	int mEdgeIDOffsets[12];		// used to obtain edge ID from vertex ID and edge number
	float mIsolevel;			// isosurface level
	VertexAction * mVertexAction;
	bool mValidSurface;			// indicates whether a valid surface is present
	bool mComputeNormals;		// whether to compute normals
	bool mNormalize;			// whether to normalize normals
	bool mInBox;

	EdgeVertex calcIntersection(int nX, int nY, int nZ, int nEdgeNo, const float * vals) const;
	void addEdgeVertex(int x, int y, int z, int cellID, int edge, const float * vals);

	void compressTriangles();
};



// Implementation ______________________________________________________________

template <class T>
void Isosurface::generate(const T * vals){
	inBox(true);
	begin();

	int Nx = mNF[0];
	int Nxy = Nx*mNF[1];

	// iterate through cubes (not field points)
	//for(int z=0; z < mNF[2]-1; ++z){
	// support transparency (assumes higher indices are farther away)
	for(int z=mNF[2]-2; z>=0; --z){
		int z0 = z   *Nxy;
		int z1 =(z+1)*Nxy;
		for(int y=0; y < mNF[1]-1; ++y){
			int y0 = y   *Nx;
			int y1 =(y+1)*Nx;

			int z0y0 = z0+y0;
			int z0y1 = z0+y1;
			int z1y0 = z1+y0;
			int z1y1 = z1+y1;

			int z0y0_1 = z0y0+1;
			int z0y1_1 = z0y1+1;
			int z1y0_1 = z1y0+1;
			int z1y1_1 = z1y1+1;

			for(int x=0; x < mNF[0]-1; ++x){

				float v8[] = {
					float(vals[z0y0 + x]), float(vals[z0y0_1 + x]),
					float(vals[z0y1 + x]), float(vals[z0y1_1 + x]),
					float(vals[z1y0 + x]), float(vals[z1y0_1 + x]),
					float(vals[z1y1 + x]), float(vals[z1y1_1 + x])
				};

				int i3[] = {x,y,z};

				addCell(i3,	v8);
			}
		}
	}

	end();
}

} // al::

#endif
