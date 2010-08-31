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
#include "types/al_Buffer.hpp"
#include "protocol/al_Graphics.hpp"

namespace al{


struct IsosurfaceHashInt{ 
	size_t operator()(int v) const { return v; }
//	size_t operator()(int v) const { return v*2654435761UL; }
};

struct PointID {
	int newID;
	double x, y, z;
};


/// Isosurface using marching cubes
template<class T=double>
class Isosurface : public gfx::GraphicsData{
public:

	Isosurface(const T& level=T(0));
	virtual ~Isosurface();
	
	/// Get isolevel
	T level() const { return mIsolevel; }

	/// Returns true if a valid surface has been generated.
	bool validSurface() const { return mValidSurface; }

	/// Returns the length, width, and height of the volume in which the
	/// isosurface in enclosed in.  Returns -1 if the surface is not
	/// valid.
	int volumeLengths(double& volLengthX, double& volLengthY, double& volLengthZ) const;

	/// Sets the dimensions of the lattice
	Isosurface& cellDims(int nx, int ny, int nz);

	/// Set lengths of cell
	Isosurface& cellLengths(double dx, double dy, double dz);

	/// Set isolevel
	Isosurface& level(T v){ mIsolevel=v; return *this; }

	/// Begin cell-at-a-time mode
	void begin();
	
	/// End cell-at-a-time mode
	void end();

	/// Add a cell from a scalar field
	
	/// This should be called in between calls to begin() and end().
	///
	void addCell(
		const T& xyz, const T& Xyz,
		const T& xYz, const T& XYz,
		const T& xyZ, const T& XyZ,
		const T& xYZ, const T& XYZ,
		int ix, int iy, int iz
	);

	template <class Ti>
	void addCell(const T * v, const Ti * i){
		addCell(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],i[0],i[1],i[2]);
	}

	/// Clear the isosurface
	void clear();

	/// Generate isosurface from scalar field contained in the buffer scalarField[]
	
	/// The number of elements in the field is expected to be nX*nY*nZ.
	///
	void generate(const T* scalarField, int nX, int nY, int nZ, float cellLengthX, float cellLengthY, float cellLengthZ);

	void generate(const T* scalarField, int n, float cellLength){
		generate(scalarField, n,n,n, cellLength,cellLength,cellLength);
	}


protected:	
	struct TRIANGLE{ int indices[3]; };
	typedef std::tr1::unordered_map<int, PointID, IsosurfaceHashInt> ID2PointID;
//	typedef std::hash_map<int, PointID, IsosurfaceHashInt> ID2PointID;
//	typedef std::map<int, PointID> ID2PointID;

	ID2PointID mID2PointID;				// Map of POINT3Ds which form the isosurface
	al::Buffer<TRIANGLE> mTriangles;	// Array of TRIANGLES indices which form the triangulation of the isosurface

	int mN1, mN2, mN3;				// No. of cells in x, y, and z directions
	int mN1p, mN2p;
	int mEdgeIDOffsets[12];
	double mL1, mL2, mL3;			// Cell length in x, y, and z directions
	T mIsolevel;					// The isosurface value
	bool mValidSurface;				// Indicates whether a valid surface is present

	int edgeID(int vertID, int edgeNo) const;
	int vertexID(int ix, int iy, int iz) const;
	PointID calcIntersection(int nX, int nY, int nZ, int nEdgeNo, const T * cellVals) const;
	void renameVerticesAndTriangles();
	void calcNormals();
	void addEdgeVertex(int x, int y, int z, int vertID, int edge, const T * cellVals);
};


} // al::

#endif
