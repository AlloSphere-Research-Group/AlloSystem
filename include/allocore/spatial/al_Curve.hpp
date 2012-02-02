#ifndef INCLUDE_AL_CURVE_HPP
#define INCLUDE_AL_CURVE_HPP

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
	Utilities for computing features of curves

	File author(s):
	Lance Putnam, 2012, putnam.lance@gmail.com
*/

namespace al{

/// Frenet frame generator

/// A Frenet frame is an orthonormal reference frame describing the local 
/// tangent (T), normal (N), and binormal (B) unit vectors of a curve. The frame
/// is based on a right-handed coordinate system so that T = N x B, N = B x T, 
/// and B = T x N.
/// \tparam Vec3	3-vector type
template <class Vec3>
struct Frenet{

	Vec3 T;		///< Tangent vector of current frame
	Vec3 N;		///< Normal vector of current frame
	Vec3 B;		///< Binormal vector of current frame


	/// @param[in] p2	two points back
	/// @param[in] p1	one point back
	Frenet(const Vec3& p2=Vec3(0,0,0), const Vec3& p1=Vec3(0,0,0)){
		init(p2,p1);
	}

	/// Get point one ahead of currently stored frame
	const Vec3& point(){ return mp1; }

	/// Get backward first difference
	const Vec3& db(){ return mdb; }

	/// Get forward first difference
	const Vec3& df(){ return mdf; }

	/// Get (central) second difference
	const Vec3& d2(){ return md2; }


	/// Compute Frenet frame one point back from input point
	void operator()(const Vec3& p0){ next<1,1,1,1>(p0); }

	/// Compute Frenet frame one point back from input point
	
	/// For best results, avoid runs of 3 or more colinear points and 2 or more
	/// duplicate points. Colinear points result in ambiguous normal and 
	/// binormal vectors. Duplicated points make the first derivative undefined.
	/// Both of these situations are guaranteed to wreak numerical havok.
	template<bool NormalizeT, bool NormalizeN, bool NormalizeB, bool ComputeN>
	void next(const Vec3& p0){

		mdb = mdf;			// bwd diff is previous fwd diff
		mdf = p0 - mp1;		// compute new fwd diff
		mp1 = p0;			// store previous

		// Consecutive points equal? If so, use previous frame...
		//if(mdb == Vec3(0) || mdf == Vec3(0)) return;

		md2 = mdf - mdb;	// second diff

		T = mdb + mdf;		// central diff
							// (really half this, but we only need eigenvector)
		if(NormalizeT) T.normalize();

//		// Colinear? Don't bother checking for now...
//		if(0){
//			if(angle(mdf, mdb) < 0.001){
//				//printf("bail\n");
//				return;
//			}
//		}

		B = cross(T, md2);

		if(ComputeN){
			N = cross(B, T);
			if(NormalizeN) N.normalize();
		}
		
		if(NormalizeB) B.normalize();
	}

//	/// Get curvature one back back from previous input point
//	value_type curvature() const {
//		Vec3 d1 = (mdf + mdb) * 0.5;
//		value_type d1MagSqr = d1.magSqr();
//		return sqrt(cross(d1, md2).magSqr() / (d1MagSqr*d1MagSqr*d1MagSqr));
//	}

	/// (Re)initialize with previous two points
	void init(const Vec3& p2, const Vec3& p1){
		mdf = p1 - p2;
		mp1 = p1;
	}

protected:
	Vec3 mp1;	// Previously input point
	Vec3 mdb;	// Backward first difference
	Vec3 mdf;	// Forward first difference
	Vec3 md2;	// (Central) second difference
};


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
