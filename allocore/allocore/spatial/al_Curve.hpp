#ifndef INCLUDE_AL_CURVE_HPP
#define INCLUDE_AL_CURVE_HPP

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
	Utilities for computing features of curves

	File author(s):
	Lance Putnam, 2012, putnam.lance@gmail.com
*/

namespace al{

/// Compute Frenet frame (tangent, normal) from 1st difference
template <class V2>
void frenet(const V2& d1, V2& t, V2& n);

/// Compute Frenet frame (tangent, normal, binormal) from 1st and 2nd differences
template <class V3>
void frenet(const V3& d1, const V3& d2, V3& t, V3& n, V3& b);

/// Compute Frenet frame (tangent, normal, binormal) from 3 consecutive points
template <class V3>
void frenet(const V3& p2, const V3& p1, const V3& p0, V3& t, V3& n, V3& b);



/// Frenet frame generator

/// This class generates a sequence of Frenet frames given a corresponding
/// sequence of discrete points along a curve. A Frenet frame is computed from
/// a run of three consecutive points and corresponds to the middle point.
/// A Frenet frame is an orthonormal reference frame describing the local
/// tangent (T), normal (N), and binormal (B) unit vectors of a curve. The frame
/// is based on a right-handed coordinate system so that T = N x B, N = B x T,
/// and B = T x N.
/// \tparam Vec3	3-vector type
///
/// @ingroup allocore
template <class Vec3>
struct Frenet{

	Vec3 T;		///< Tangent vector of current frame
	Vec3 N;		///< Normal vector of current frame
	Vec3 B;		///< Binormal vector of current frame


	/// @param[in] p2	two points back
	/// @param[in] p1	one point back
	Frenet(const Vec3& p2=Vec3(0,-0.01,0), const Vec3& p1=Vec3(0,0,0)){
		init(p2,p1);
	}

	/// Get point one ahead of currently stored frame
	const Vec3& point() const { return mp1; }

	/// Get backward first difference
	const Vec3& db() const { return mdb; }

	/// Get forward first difference
	const Vec3& df() const { return mdf; }

	/// Get (central) second difference
	Vec3 d2() const { return mdf - mdb; }


	/// Compute Frenet frame one point back from input point
	void operator()(const Vec3& p0){ next<1,1,1,1,1>(p0); }

	/// Compute Frenet frame one point back from input point

	/// The input points must be "curved" to generate a valid frame. This means 
	/// no duplicate points or runs of 3 or more colinear points. Duplicated
	/// points make the tangent undefined. Colinear points result in ambiguous 
	/// normal and binormal vectors.
	template<bool NormalizeT, bool NormalizeN, bool NormalizeB, bool ComputeN, bool ComputeB>
	void next(const Vec3& p0){

		mdb = mdf;			// bwd diff is previous fwd diff
		mdf = p0 - mp1;		// compute new fwd diff
		mp1 = p0;			// store previous

		// Consecutive points equal? If so, use previous frame...
		//if(mdb == Vec3(0) || mdf == Vec3(0)) return;

		T = mdb + mdf;		// central diff
							// (really half this, but we only need eigenvector)
		if(NormalizeT) T.normalize();

		// Colinear? If so, use previous binormal and normal...
		/*if(angle(mdf, mdb) < 0.001){
			//printf("colinear\n");
			return;
		}*/

		if(ComputeB || ComputeN){
			B = cross(mdb, mdf);
			//B = cross(T, mdf - mdb); // formally, we use 2nd difference
		}

		if(ComputeN){
			N = cross(B, T);
			if(NormalizeN) N.normalize();
		}

		if(NormalizeB) B.normalize();
	}

	/// (Re)initialize with previous two points
	void init(const Vec3& p2, const Vec3& p1){
		mdf = p1 - p2;
		mp1 = p1;
	}

//	/// Get curvature one back back from previous input point
//	value_type curvature() const {
//		Vec3 d1 = (mdf + mdb) * 0.5;
//		value_type d1MagSqr = d1.magSqr();
//		return sqrt(cross(d1, md2).magSqr() / (d1MagSqr*d1MagSqr*d1MagSqr));
//	}

protected:
	Vec3 mp1;	// Previously input point
	Vec3 mdb;	// Backward first difference
	Vec3 mdf;	// Forward first difference
};



// Implementation

template <class V2>
inline void frenet(const V2& d1, V2& t, V2& n){
	t = d1;
	t.normalize();
	// normal according to right-hand rule
	n[0] =-t[1];
	n[1] = t[0];
}

template <class V3>
inline void frenet(const V3& d1, const V3& d2, V3& t, V3& n, V3& b){
	b = cross(d2, d1);
	n = cross(d1, b);
	t = d1;
	t.normalize();
	b.normalize();
	n.normalize();
}

template <class V3>
inline void frenet(const V3& p2, const V3& p1, const V3& p0, V3& t, V3& n, V3& b){
	V3 d1 = (p0 - p2);			// 1st (central) difference (scaled by 2)
	V3 d2 = (p0 - p1*2. + p2);	// 2nd difference
	frenet(d1,d2, t,n,b);
}



//
///// Compute curvature around point b of three successive points a, b, and c.
//template <class T, template <class> class NVec>
//T curvature(const NVec<T>& a, const NVec<T>& b, const NVec<T>& c);
//
//template <class T, template <class> class V>
//T curvature(const V<T>& a, const V<T>& b, const V<T>& c){
//
//	V<T> d1b = b-a;				// first backward difference
//	V<T> d1f = c-b;				// first forward difference
//	V<T> d1  = (d1f+d1b) * 0.5;	// first mid difference
//
//	V<T> d2  = d1f - d1b;		// second difference
//
//	T d1n = d1.norm();
//
//	return (d1.cross(d2)).norm() / (d1n*d1n*d1n);
//}


} // al::
#endif
