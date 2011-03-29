#ifndef INCLUDE_AL_ISOSURFACE_HPP
#define INCLUDE_AL_ISOSURFACE_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
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
*/

#include <map>
#include <vector>
#if defined(WIN32) || defined(__WINDOWS_MM__)
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include "allocore/types/al_Buffer.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{


/// Isosurface generated using marching cubes
class Isosurface : public Mesh {
public:

	struct EdgeVertex {
		Vec3i pos;			// cell coordinates
		//int idx;			// cell flattened array index
		//int corners[2];		// edge corners as offsets from cell position
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


	Isosurface(float level=0, VertexAction * a=0);

	virtual ~Isosurface();

	int fieldDim(int i){ return mNF[i]; }

	/// Get isolevel
	float level() const { return mIsolevel; }

	/// Returns true if a valid surface has been generated.
	bool validSurface() const { return mValidSurface; }

	/// Returns the length, width, and height of the volume in which the
	/// isosurface in enclosed in.  Returns -1 if the surface is not
	/// valid.
	int volumeLengths(double& volLengthX, double& volLengthY, double& volLengthZ) const;

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

	/// Clear the isosurface
	void clear();


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
	
	/// The number of elements in the field is expected to be nX*nY*nZ.
	///
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

	void vertexAction(VertexAction& a){ mVertexAction = &a; }

protected:

	struct IsosurfaceHashInt{ 
		size_t operator()(int v) const { return v; }
	//	size_t operator()(int v) const { return v*2654435761UL; }
	};

	typedef std::tr1::unordered_map<int, int, IsosurfaceHashInt> EdgeToVertex;
//	typedef std::tr1::unordered_map<int, VertexData, IsosurfaceHashInt> EdgeToVertex;
//	typedef std::hash_map<int, VertexData, IsosurfaceHashInt> EdgeToVertex;
//	typedef std::map<int, VertexData> EdgeToVertex;

	EdgeToVertex mEdgeToVertex;					// map from edge ID to vertex
	al::Buffer<EdgeTriangle> mEdgeTriangles;	// surface triangles in terms of edge IDs
	
	al::Buffer<int> mTempEdges;

	double mL[3];				// cell length in x, y, and z directions
	int mNF[3];					// number of field points in x, y, and z directions
	int mEdgeIDOffsets[12];		// used to obtain edge ID from vertex ID and edge number
	float mIsolevel;			// isosurface level
	VertexAction * mVertexAction;
	bool mValidSurface;			// indicates whether a valid surface is present
	bool mComputeNormals;		// whether to compute normals
	bool mNormalize;			// whether to normalize normals

	
	EdgeVertex calcIntersection(int nX, int nY, int nZ, int nEdgeNo, const float * vals) const;
	void addEdgeVertex(int x, int y, int z, int cellID, int edge, const float * vals);
	
	void compressTriangles();
};





// Implementation ______________________________________________________________

template <class T>
void Isosurface::generate(const T * vals){
	begin();

	int Nx = mNF[0];
	int Nxy = Nx*mNF[1];

	// iterate through cubes (not field points)
	for(int z=0; z < mNF[2]-1; ++z){
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
			vals[z0y0 + x], vals[z0y0_1 + x],
			vals[z0y1 + x], vals[z0y1_1 + x],
			vals[z1y0 + x], vals[z1y0_1 + x],
			vals[z1y1 + x], vals[z1y1_1 + x]
		};

		int i3[] = {x,y,z};

		addCell(i3,	v8);
	}}}
	
	end();
}



} // al::

#endif
