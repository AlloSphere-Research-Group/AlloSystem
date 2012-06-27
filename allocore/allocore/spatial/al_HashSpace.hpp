#ifndef INCLUDE_AL_HASHSPACE_HPP
#define INCLUDE_AL_HASHSPACE_HPP

#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Array.hpp"

#include <vector>


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
	HashSpace is a way to detect object collisions using a voxel grid
	The grid has a given resolution (no. voxel cells per side)
	
	It is optimized for densely packed points and querying for nearest neighbors
	within given radii (results will be roughly sorted by distance). 
	
	TODO: non-toroidal options
	TODO: have query() automatically (insertion) sort results by distance
		(perhaps use std::set instead of vector?)

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Graham Wakefield, 2011, grrrwaaa@gmail.com
	
	Inspired by this paper:
	http://nicolas.brodu.numerimoire.net/common/recherche/publications/QuerySphereIndexing.pdf
*/

namespace al {

/**
	HashSpace is a way to detect object collisions using a voxel grid
	The grid has a given resolution (no. voxel cells per side)
	
	It is optimized for densely packed points and querying for nearest neighbors
	within given radii (results will be roughly sorted by distance). 
*/
class HashSpace {
public:
	
	/// container for registered spatial elements
	struct Object {
		Object() : hash(invalidHash()), next(NULL), prev(NULL), userdata(0) {}
			
		Vec3d pos;		
		uint32_t hash;	///< which voxel ID it belongs to (or invalidHash())
		Object * next, * prev;	///< neighbors in the same voxel
		union {			///< a way to attach user-defined payloads:
			uint32_t id;
			void * userdata;
		};
	};

	/// each Voxel contains a linked list of Objects
	struct Voxel {
		Voxel() : mObjects(NULL) {}
		Voxel(const Voxel& cpy) : mObjects(NULL) {}
		
		/// definitely not thread-safe.
		inline void add(Object * o);
		
		/// definitely not thread-safe.
		inline void remove(Object * o);
		
		/// the linked list of objects in this voxel
		Object * mObjects;
	};
	
	/** 
		Query functor 
		create and re-use a query functor to find neighbors
		
		Query query;
		
		query.clear(); // do this if you want to re-use the query object
		query(space, Vec3d(0, 0, 0), 10);
		for (int i=0; i<query.size(); i++) {
			Object * o = query[i];
			...
		}
	*/
	struct Query {
		
		struct Result {
			HashSpace::Object * object;
			double distanceSquared;
			
			static bool compare(const Result& x, const Result& y) {
				return x.distanceSquared > y.distanceSquared;
			}
			
			Result() : object(0), distanceSquared(0) {}
			Result(const Result& cpy) : object(cpy.object), distanceSquared(cpy.distanceSquared) {}
		};
		
		typedef std::vector<Result> Results;
		typedef Results::iterator Iterator;
	
		/** 
			Constructor
			@param maxResults the maximum number of results to find
		*/
		Query(uint32_t maxResults=128) 
		:	mMaxResults(maxResults)
		{
			mObjects.reserve(maxResults);
		}
		
		/** 
			The main method of the query object
			finds the neighbors of a given point, within given distances
			the matches will be roughly (not exactly) sorted by distance
			
			@param space the HashSpace object to search in
			@param center finds objects near to this point
			@param obj finds objects near to this object
			@param maxRadius finds objects if they are nearer this distance
				the maximum permissible value of radius is space.maxRadius()
			@param minRadius finds objects if they are beyond this distance
			@return the number of results found
		*/ 
		int operator()(const HashSpace& space, const Vec3d center, double maxRadius, double minRadius=0.);
		int operator()(const HashSpace& space, const Object * obj, double maxRadius, double minRadius=0.);
		
		/**
			finds all the neighbors of the given point, up to maxResults()
			the matches will be roughly (not exactly) sorted by distance
			
			@param space the HashSpace object to search in
			@param center finds objects near to this point
			@param obj finds objects near to this object
		*/
		int operator()(const HashSpace& space, Vec3d center);
		int operator()(const HashSpace& space, const Object * obj);
		
		/** 
			finds nearest neighbor of an object
		*/
		Object * nearest(const HashSpace& space, const Object * obj);
		
		
		/// get number of results:
		unsigned size() const { return mObjects.size(); }
		/// get each result:
		Object * operator[](unsigned i) const { return mObjects[i].object; }
		double distanceSquared(unsigned i) const { return mObjects[i].distanceSquared; }
		double distance(unsigned i) const { return sqrt(distanceSquared(i)); }
		
		/** 
			clear is separated from the main query operation, 
			to support aggregating queries in series
			typically however you would want to clear() and then call the query.
		*/
		Query& clear() { mObjects.clear(); return *this; }
		
		/// set the maximum number of desired results
		Query& maxResults(uint32_t i) { mMaxResults = i; return *this; }
		/// get the maximum number of desired results
		uint32_t maxResults() const { return mMaxResults; }
		
		/// std::vector interface:
		Iterator begin() { return mObjects.begin(); }
		Iterator end() { return mObjects.end(); }
		Results& results() { return mObjects; }
		
	protected:
		uint32_t mMaxResults;
		Results mObjects;
	};
	
	/** 
		Construct a HashSpace
		locations will range from [0..2^resolution)
		
		resolution can be 1 to 10; the dim is 2^resolution i.e. 2..1024
		(the limit is 10 so that the hash can fit inside a uint32_t integer)
		default 5 implies 32 units per side

		
		@param resolution determines the number of voxels as 2^resolution per axis 
		@param numObjects set how many Object slots to initally allocate
	*/
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
	template<typename T>
	HashSpace& move(uint32_t objectId, Vec<3,T> pos);
	
	/// this removes the object from voxels/queries, but does not destroy it
	/// the objectId can be reused later via move()
	HashSpace& remove(uint32_t objectId);
	
	/// wrap an absolute position within the space:
	double wrap(double x) const { return wrap(x, dim()); }
	template<typename T>
	Vec<3,T> wrap(Vec<3,T> v) const {
		return Vec<3,T>(wrap(v.x), wrap(v.y), wrap(v.z));
	}
	
	/// wrap a relative vector within the space:
	/// use this when computing the vector between objects
	/// to properly take into account toroidal wrapping
	double wrapRelative(double x) const { return wrap(x, maxRadius()); }
	template<typename T>
	Vec<3,T> wrapRelative(Vec<3,T> v) const {
		return wrap(v + maxRadius()) - maxRadius();
	}

	/// an invalid voxel index used to indicate non-membership
	static uint32_t invalidHash() { return UINT_MAX; }
	
protected:	
		
	// integer distance squared
	uint32_t distanceSquared(double a1, double a2, double a3) const;

	// convert x,y,z in range [0..DIM) to unsigned hash:
	// this is also the valid mVoxels index for the corresponding voxel:
	inline uint32_t hash(unsigned x, unsigned y, unsigned z) const { 
		return hashx(x)+hashy(y)+hashz(z); 
	}
	template<typename T>
	inline uint32_t hash(Vec<3,T> v) const { return hash(v[0], v[1], v[2]); }
	// generate hash offset by an already generated hash:
	inline uint32_t hash(uint32_t x, uint32_t y, uint32_t z, uint32_t offset) const {
		return	hashx(unhashx(offset) + x) + 
				hashy(unhashy(offset) + y) + 
				hashz(unhashz(offset) + z);
	}
	template<typename T>
	inline uint32_t hash(Vec<3,T> v, uint32_t offset) const { 
		return hash(v[0], v[1], v[2], offset); 
	}
	
	
	inline uint32_t hashx(uint32_t v) const { return v & mWrap; }
	inline uint32_t hashy(uint32_t v) const { return (v & mWrap)<<mShift; }
	inline uint32_t hashz(uint32_t v) const { return (v & mWrap)<<mShift2; }
	
	// convert unsigned hash to x,y,z in range [0..mDim):
	inline Vec3i unhash(uint32_t h) const { return Vec3i(unhashx(h), unhashy(h), unhashz(h)); }
	
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
	std::vector<uint32_t> mVoxelIndicesToDistance;
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

inline int HashSpace::Query :: operator()(const HashSpace& space, const Object * obj) {
	return (*this)(space, obj, space.maxRadius());
}

// the maximum permissible value of radius is mDimHalf
// if int(inner^2) == int(outer^2), only 1 shell will be queried.
// TODO: non-toroidal version.
inline int HashSpace::Query :: operator()(const HashSpace& space, Vec3d center, double maxRadius, double minRadius) {
	unsigned nres = 0;
	double minr2 = minRadius*minRadius;
	double maxr2 = maxRadius*maxRadius;
	uint32_t iminr2 = al::max(uint32_t(0), uint32_t(minRadius*minRadius));
	uint32_t imaxr2 = al::min(space.mMaxHalfD2, uint32_t(1 + (maxRadius+1)*(maxRadius+1)));
	if (iminr2 < imaxr2) { 
		uint32_t cellstart = space.mDistanceToVoxelIndices[iminr2];
		uint32_t cellend = space.mDistanceToVoxelIndices[imaxr2];
		for (uint32_t i = cellstart; i < cellend; i++) {
			uint32_t index = space.hash(center, space.mVoxelIndices[i]);
			const Voxel& voxel = space.mVoxels[index];
			// now add any objects in this voxel to the result... 
			Object * head = voxel.mObjects;
			if (head) {
				Object * o = head;
				do {
					// final check - float version:
					Vec3d rel = space.wrapRelative(o->pos - center);
					double d2 = rel.magSqr();
					if (d2 >= minr2 && d2 <= maxr2) {
						mObjects[nres].object = o;
						mObjects[nres].distanceSquared = d2;
						nres++;
					}
					o = o->next;
				} while (o != head && nres < mMaxResults);
			}
			if(nres == mMaxResults) {
				break;
			}
		}
	}
	//std::sort(mObjects.begin(), mObjects.end(), Result::compare);
	return nres;
}

// the maximum permissible value of radius is mDimHalf
// if int(inner^2) == int(outer^2), only 1 shell will be queried.
// TODO: non-toroidal version.
inline int HashSpace::Query :: operator()(const HashSpace& space, const HashSpace::Object * obj, double maxRadius, double minRadius) {
	unsigned nres = 0;
	const Vec3d& center = obj->pos;
	double minr2 = minRadius*minRadius;
	double maxr2 = maxRadius*maxRadius;
	uint32_t iminr2 = al::max(uint32_t(0), uint32_t(minRadius*minRadius));
	uint32_t imaxr2 = al::min(space.mMaxHalfD2, uint32_t(1 + (maxRadius+1)*(maxRadius+1)));
	if (iminr2 < imaxr2) { 
		uint32_t cellstart = space.mDistanceToVoxelIndices[iminr2];
		uint32_t cellend = space.mDistanceToVoxelIndices[imaxr2];
		for (uint32_t i = cellstart; i < cellend; i++) {
			uint32_t index = space.hash(center, space.mVoxelIndices[i]);
			const Voxel& voxel = space.mVoxels[index];
			// now add any objects in this voxel to the result... 
			Object * head = voxel.mObjects;
			if (head) {
				Object * o = head;
				do {
					if (o != obj) {
						// final check - float version:
						Vec3d rel = space.wrapRelative(o->pos - center);
						double d2 = rel.magSqr();
						if (d2 >= minr2 && d2 <= maxr2) {
							// here we could insert-sort based on distance...
							mObjects[nres].object = o;
							mObjects[nres].distanceSquared = d2;
							nres++;
						}
					}
					o = o->next;
				} while (o != head && nres < mMaxResults);
			}
			if(nres == mMaxResults) break;
		}
	}
	//std::sort(mObjects.begin(), mObjects.end(), Result::compare);
	return nres;
}

// of the matches, return the best:
inline HashSpace::Object * HashSpace::Query :: nearest(const HashSpace& space, const Object * src) {
	clear();
	const Vec3d& center = src->pos;
	uint32_t results = (*this)(space, src, space.mMaxHalfD2);
	
	Object * result = 0;
	double rd2 = space.mMaxHalfD2;
	for (uint32_t i=0; i<results; i++) {
		Object * o = mObjects[i].object;
		Vec3d rel = space.wrapRelative(o->pos - center);
		double d2 = rel.magSqr();
		if (d2 < rd2) {
			rd2 = d2;
			result = o;
		}
	}
	return result;
}

	
inline void HashSpace :: numObjects(int numObjects) {
	mObjects.clear();
	mObjects.resize(numObjects);
	// clear all voxels:
	for (unsigned i=0; i<mVoxels.size(); i++) {
		mVoxels[i].mObjects = 0;
	}
}

template<typename T>
inline HashSpace& HashSpace :: move(uint32_t objectId, Vec<3,T> pos) {
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
