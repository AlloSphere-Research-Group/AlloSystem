#ifndef INCLUDE_AL_HASHSPACE_HPP
#define INCLUDE_AL_HASHSPACE_HPP

#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Array.hpp"

#include <vector>


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
	HashSpace is a way to detect object collisions using a voxel grid
	The grid has a given resolution (no. voxel cells per side)
	
	It is optimized for densely packed points and querying for nearest neighbors
	within given radii (results will be roughly sorted by distance). 
	
	TODO: non-toroidal version.

	File author(s):
	Graham Wakefield, 2011, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
	
	Inspired by this paper:
	http://nicolas.brodu.numerimoire.net/common/recherche/publications/QuerySphereIndexing.pdf
*/

namespace al {

class HashSpace {
public:
	
	// container for registered spatial elements
	struct Object {
		Object() : hash(invalidHash()), next(NULL), prev(NULL), userdata(0) {}
			
		Vec3d pos;		
		uint32_t hash;	///< which voxel ID it belongs to (or invalidHash())
		Object * next, * prev;	///< neighbors in the same voxel
		union {			// a way to attach user-defined payloads:
			uint32_t id;
			void * userdata;
		};
	};

	struct Voxel {
		Voxel() : mObjects(NULL) {}
		Voxel(const Voxel& cpy) : mObjects(NULL) {}
		
		// this is definitely not thread-safe.
		inline void add(Object * o);
		
		// this is definitely not thread-safe.
		inline void remove(Object * o);
		
		Object * mObjects;
	};
	
	// create and re-use a query object to find neighbors:
	struct Query {
	
		typedef std::vector<HashSpace::Object *> Vector;
		typedef Vector::iterator Iterator;
	
		Query(uint32_t maxResults=128) 
		:	mMaxResults(maxResults)
		{
			mObjects.reserve(maxResults);
		}
		
		/// clear is separated from the main query operation, 
		/// to support aggregating queries in series
		/// typically however you would want to clear() and then query.
		Query& clear() { mObjects.clear(); return *this; }
		
		/// set/get the maximum number of desired results
		Query& maxResults(uint32_t i) { mMaxResults = i; return *this; }
		uint32_t maxResults() const { return mMaxResults; }
		
		/// get the neighbors within maxRadius (and beyond minRadius if given)
		// the maximum permissible value of radius is space.maxRadius()
		int operator()(const HashSpace& space, const Vec3d center, double maxRadius, double minRadius=0.);
		
		/// just get the nearest neighbors
		int operator()(const HashSpace& space, Vec3d center);
		
		/// get number of results:
		unsigned size() const { return mObjects.size(); }
		/// get each result:
		Object * operator[](unsigned i) const { return mObjects[i]; }
		
		// std::vector interface:
		Iterator begin() { return mObjects.begin(); }
		Iterator end() { return mObjects.end(); }
		Vector& vector() { return mObjects; }
		
	protected:
		uint32_t mMaxResults;
		std::vector<Object *> mObjects;
	};
	
	/// resolution defines the size of the space (2^resolution per axis)
	/// locations will range from [0..2^resolution)
	HashSpace(uint32_t resolution=5, uint32_t numObjects=0);
	~HashSpace();
	
	/// the dimension of the space per axis:
	uint32_t dim() const { return mDim; }
	/// the maximum valid radius to query (half the dimension):
	uint32_t maxRadius() const { return mDimHalf; }
	
	
	/// get/set the number of objects:
	void numObjects(int numObjects);
	uint32_t numObjects() const { return mObjects.size(); }
	
	/// get the object at a given index:
	Object& object(uint32_t i) { return mObjects[i]; }
	
	/// set the position of an object:
	HashSpace& move(uint32_t objectId, double x, double y, double z) { return move(objectId, Vec3d(x,y,z)); }
	HashSpace& move(uint32_t objectId, Vec3d pos);
	
	/// this removes the object from voxels/queries, but does not destroy it
	/// the objectId can be reused later via move()
	HashSpace& remove(uint32_t objectId);
	
	
	/// wrap an absolute position within the space:
	double wrap(double x) const { return wrap(x, dim()); }
	Vec3d wrap(Vec3d v) const {
		return Vec3d( wrap(v.x), wrap(v.y), wrap(v.z) );
	}
	
	/// wrap a relative vector within the space:
	/// use this when computing the vector between objects
	/// to properly take into account toroidal wrapping
	double wrapRelative(double x) const { return wrap(x, maxRadius()); }
	Vec3d wrapRelative(Vec3d v) const {
		return wrap(v + maxRadius()) - maxRadius();
	}

	/// an invalid voxel index used to indicate non-membership
	static uint32_t invalidHash() { return UINT_MAX; }
	
protected:	
		
	// integer distance squared
	uint32_t distanceSquared(double a1, double a2, double a3) const;

	// convert x,y,z in range [0..DIM) to unsigned hash:
	// this is also the valid mVoxels index for the corresponding voxel:
	template<typename T>
	uint32_t hash(Vec<3,T> v) const { return hash(v[0], v[1], v[2]); }
	uint32_t hash(unsigned x, unsigned y, unsigned z) const { 
		return hashx(x)+hashy(y)+hashz(z); 
	}
	
	inline uint32_t hashx(uint32_t v) const { return v & mWrap; }
	inline uint32_t hashy(uint32_t v) const { return (v & mWrap)<<mShift; }
	inline uint32_t hashz(uint32_t v) const { return (v & mWrap)<<mShift2; }
	
	// convert unsigned hash to x,y,z in range [0..mDim):
	Vec3i unhash(uint32_t h) const { return Vec3i(unhashx(h), unhashy(h), unhashz(h)); }
		
	inline uint32_t unhashx(uint32_t h) const { return (h) & mWrap; }
	inline uint32_t unhashy(uint32_t h) const { return (h>>mShift) & mWrap; }
	inline uint32_t unhashz(uint32_t h) const { return (h>>mShift2) & mWrap; }
	
	// safe floating-point wrapping
	static double wrap(double x, double mod);
	static double wrap(double x, double lo, double hi);

	uint32_t mShift, mShift2, mDim, mDim2, mDim3, mWrap, mWrap3;
	int mDimHalf;	// the valid maximum radius for queries
	uint32_t mMaxD2, mMaxHalfD2;
	
	/// the array of objects
	std::vector<Object> mObjects;
	
	/// the array of voxels (indexed by hashed location)
	std::vector<Voxel> mVoxels;	
		
	/// a baked array of voxel indices sorted by distance 
	std::vector<uint32_t> mVoxelIndices;
	/// a baked array mapping distance to mVoxelIndices offsets 
	std::vector<uint32_t> mDistanceToVoxelIndices;
};


		
// this is definitely not thread-safe.
inline void HashSpace::Voxel :: add(Object * o) {
	if (mObjects) {
		// add to tail:
		Object * last = mObjects->prev;
		last->next = o;
		o->prev = last;
		o->next = mObjects;
		mObjects->prev = o;
	} else {
		// unique:
		mObjects = o->prev = o->next = o; 
	}
}

// this is definitely not thread-safe.
inline void HashSpace::Voxel :: remove(Object * o) {
	if (o == o->prev) {	// voxel only has 1 item
		mObjects = NULL;
	} else {
		Object * prev = o->prev;
		Object * next = o->next;
		prev->next = next;
		next->prev = prev;
		// update head pointer?
		if (mObjects == o) { mObjects = next; }
	}
	// leave the object clean:
	o->prev = o->next = NULL;
}

inline int HashSpace::Query :: operator()(const HashSpace& space, Vec3d center) {
	return (*this)(space, center, space.maxRadius());
}
	
// the maximum permissible value of radius is mDimHalf
// if int(inner^2) == int(outer^2), only 1 shell will be queried.
// TODO: non-toroidal version.
inline int HashSpace::Query :: operator()(const HashSpace& space, Vec3d center, double maxRadius, double minRadius) {
	unsigned nres = 0;
	uint32_t offset = space.hash(center);
	double minr2 = minRadius*minRadius;
	double maxr2 = maxRadius*maxRadius;
	uint32_t iminr2 = al::max(uint32_t(0), uint32_t((minRadius-1)*(minRadius-1)));
	uint32_t imaxr2 = al::min(space.mMaxHalfD2, uint32_t((maxRadius+1)*(maxRadius+1)));
	if (iminr2 < imaxr2) { 
		uint32_t cellstart = space.mDistanceToVoxelIndices[iminr2];
		uint32_t cellend = space.mDistanceToVoxelIndices[imaxr2];
		
		for (uint32_t i = cellstart; i < cellend; i++) {
			// offset current index by voxel grid:
			uint32_t index = (offset + space.mVoxelIndices[i]) & space.mWrap3;
			const Voxel& cell = space.mVoxels[index];
			// now add any objects in this voxel to the result... 
			Object * head = cell.mObjects;
			if (head) {
				Object * o = head;
				do {
					// final check - float version:
					Vec3d rel = space.wrapRelative(o->pos - center);
					double d2 = rel.magSqr();
					if (d2 >= minr2 && d2 <= maxr2) {
						mObjects.push_back(o);
						nres++;
					}
					o = o->next;
				} while (o != head && nres < mMaxResults);
			}
			if(nres == mMaxResults) break;
		}
	}
	return nres;
}
	
inline void HashSpace :: numObjects(int numObjects) {
	mObjects.clear();
	mObjects.resize(numObjects);
	// clear all voxels:
	for (unsigned i=0; i<mVoxels.size(); i++) {
		mVoxels[i].mObjects = 0;
	}
}

inline HashSpace& HashSpace :: move(uint32_t objectId, Vec3d pos) {
	Object& o = mObjects[objectId];
	o.pos.set(wrap(pos));
	uint32_t newhash = hash(o.pos);
	if (newhash != o.hash) {
		if (o.hash != invalidHash()) mVoxels[o.hash].remove(&o);
		o.hash = newhash;
		mVoxels[newhash].add(&o);
	}
	return *this;
}

inline HashSpace& HashSpace :: remove(uint32_t objectId) {
	Object& o = mObjects[objectId];
	if (o.hash != invalidHash()) mVoxels[o.hash].remove(&o);
	o.hash = invalidHash();
	return *this;
}

// integer distance squared
inline uint32_t HashSpace :: distanceSquared(double x, double y, double z) const {
	return x*x+y*y+z*z;
}

// wrapping floating point numbers is surprisingly complex
// because of rounding errors, behavior when near zero
// and other oddities
inline double HashSpace :: wrap(double x, double mod) {
	if (mod) {
		if (x > mod) {
			// shift down
			if (x > (mod*2.)) {
				// multiple wraps:
				double div = x / mod;
				// get fract:
				double divl = (long)div;
				double fract = div - (double)divl;
				return fract * mod;
			} else {
				// single wrap:
				return x - mod;
			}
		} else if (x < 0.) {
			// negative x, shift up
			if (x < -mod) {
				// multiple wraps:
				double div = x / mod;
				// get fract:
				double divl = (long)div;
				double fract = div - (double)divl;
				double x1 = fract * mod;
				return (x1 < 0.) ? x1 + mod : x1;	
			} else {
				// single wrap:
				return x + mod;
			}
		} else {
			return x;
		}
	} else {
		return 0.;	// avoid divide by zero
	}
}

inline double HashSpace :: wrap(double x, double lo, double hi) {
	return lo + wrap(x-lo, hi-lo);
}

} // al::

#endif
