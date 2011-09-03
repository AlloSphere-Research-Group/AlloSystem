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

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Graham Wakefield, 2011, grrrwaaa@gmail.com
*/

namespace al {


class HashSpace {
public:

	static uint32_t invalidHash() { return UINT_MAX; }
	
	struct Object {
		Object() : hash(invalidHash()), next(NULL), prev(NULL) {}
		
		Vec3d pos;
		uint32_t hash;
		Object * next;
		Object * prev;
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
	
	// resolution can be 1 to 10; the dim is 2^resolution i.e. 2..1024
	// (the limit is 10 so that the hash can fit inside a uint32_t integer)
	// default 5 implies 32 units per side
	HashSpace(uint32_t resolution=5, uint32_t numObjects=0);
	~HashSpace();
	
	// the maximum permissible value of radius is mDimHalf
	// if int(inner^2) == int(outer^2), only 1 shell will be queried.
	// TODO: non-toroidal version.
	int query(Vec3d center, double innerRadius, double outerRadius, std::vector<Object *>& results, int& maxresults);
	
	void rebuild(int numObjects);
	
	inline void hash(uint32_t objectId, double x, double y, double z);
	inline void hash(uint32_t objectId, Vec3d pos) { hash(objectId, pos[0], pos[1], pos[2]); }
	
	// convert x,y,z in range [0..DIM) to unsigned hash:
	// this is also the valid mVoxels index for the corresponding voxel:
	inline uint32_t hash(unsigned x, unsigned y, unsigned z) { return hashx(x)+hashy(y)+hashz(z); }
	inline uint32_t hashd(double x, double y, double z) { return hashx(x+0.5)+hashy(y+0.5)+hashz(z+0.5); }
	template<typename T>
	inline uint32_t hash(Vec3d v) { return hashd(v[0], v[1], v[2]); }
	inline uint32_t hash(Vec3f v) { return hashd(v[0], v[1], v[2]); }
	inline uint32_t hashx(uint32_t v) { return v & mWrap; }
	inline uint32_t hashy(uint32_t v) { return (v & mWrap)<<mShift; }
	inline uint32_t hashz(uint32_t v) { return (v & mWrap)<<mShift2; }
	
	// convert unsigned hash to x,y,z in range [0..mDim):
	inline uint32_t unhashx(uint32_t h) { return (h) & mWrap; }
	inline uint32_t unhashy(uint32_t h) { return (h>>mShift) & mWrap; }
	inline uint32_t unhashz(uint32_t h) { return (h>>mShift2) & mWrap; }
	
	Object& object(uint32_t i) { return mObjects[i]; }
	uint32_t numObjects() const { return mObjects.size(); }
	
	uint32_t dim() const { return mDim; }
	
	// integer distance squared
	uint32_t magSqr(int a1, int a2, int a3);
	
	uint32_t mShift, mShift2, mDim, mDim2, mDim3, mWrap, mWrap3;
	int mDimHalf;	// the valid maximum radius for queries
	uint32_t mMaxD2, mMaxHalfD2;	// squared distance from opposite corners
	
	// a 'baked' array of indices (from the origin) at given integer distances
	// i.e. each element in the array is a spherical shell
	//std::vector<std::vector<uint32_t> > mShells;
	std::vector<uint32_t> mVoxelIndices;
	std::vector<uint32_t> mDistanceToVoxelIndices;
	
	// now it is possible that we could re-arrange mVoxels to be optimally
	std::vector<Voxel> mVoxels;	// the array of voxels
	
	std::vector<Object> mObjects;
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

	
// the maximum permissible value of radius is mDimHalf
// if int(inner^2) == int(outer^2), only 1 shell will be queried.
// TODO: non-toroidal version.
inline int HashSpace :: query(Vec3d center, double innerRadius, double outerRadius, std::vector<Object *>& results, int& maxresults) {
	int nres = 0;
	int h = hash(center);
	uint32_t ir2 = innerRadius*innerRadius;
	double r2 = outerRadius*outerRadius;
	uint32_t or2 = al::min(mMaxHalfD2, uint32_t((outerRadius+1)*(outerRadius+1)));
	if (ir2 < or2) { 
		uint32_t cellstart = mDistanceToVoxelIndices[ir2];
		uint32_t cellend = mDistanceToVoxelIndices[or2];
		for (uint32_t i = cellstart; i < cellend; i++) {
			// offset current index by voxel grid:
			uint32_t index = (h + mVoxelIndices[i]) & mWrap3;
			Voxel& cell = mVoxels[index];
			// now add any objects in this voxel to the result... 
			Object * head = cell.mObjects;
			if (head) {
				Object * o = head;
				do {
					// final check - float version:
					double dx = o->pos[0] - center[0];
					double dy = o->pos[1] - center[1];
					double dz = o->pos[2] - center[2];
					double d2 = dx*dx + dy*dy + dz*dz;
					if (d2 <= r2) {
						results.push_back(o);
						nres++;
					}
					o = o->next;
				} while (o != head && nres < maxresults);
			}
			if(nres == maxresults) break;
		}
	}
	return nres;
}
	
inline void HashSpace :: rebuild(int numObjects) {
	mObjects.clear();
	mObjects.resize(numObjects);
	// clear all voxels:
	for (unsigned i=0; i<mVoxels.size(); i++) {
		mVoxels[i].mObjects = 0;
	}
}

inline void HashSpace :: hash(uint32_t objectId, double x, double y, double z) {
	Object& o = mObjects[objectId];
	o.pos.set(x, y, z);
	uint32_t newhash = hashd(x, y, z);
	if (newhash != o.hash) {
		if (o.hash != invalidHash()) mVoxels[o.hash].remove(&o);
		o.hash = newhash;
		mVoxels[newhash].add(&o);
	}
}

// integer distance squared
inline uint32_t HashSpace :: magSqr(int a1, int a2, int a3) {
	uint32_t x = al::abs(a1);
	uint32_t y = al::abs(a2);
	uint32_t z = al::abs(a3);
	return x*x+y*y+z*z;
}

//class HashSpace {
//public:
//
//	// Wrapper for an arbitary object to be stored in the space
//	class Object {
//	public:
//		typedef void (*datafree)(void *);
//		
//		Vec3f pos;
//		int oidx;		// my index in HashSpace.mObjects
//		int idx;		
//		int cellidx;	// my index in HashSpace.mCells
//		Object *next;
//		Object *prev;
//		void *data;
//		datafree dfree;
//		
//		Object()
//		:	oidx(0),
//			idx(0),
//			cellidx(HashSpace::invalidCell()),
//			next(0), 
//			prev(0),
//			data(0),
//			dfree(0)
//		{}
//		
//		Object(const Object& cpy)
//		:	oidx(0),
//			idx(0),
//			cellidx(HashSpace::invalidCell()),
//			next(0), 
//			prev(0),
//			data(0),
//			dfree(0)
//		{}
//		
//		~Object() {
//			if(dfree && data) dfree(data);
//		}
//	};
//	
//	// The voxel is a list of objects 
//	struct Cell {
//		Cell() 
//		: objects(0) {}
//		
//		Cell(const Cell& cpy) 
//		: objects(0) {}
//		
//		void addObject(Object * ob);
//		
//		Object * objects;
//	};
//	
//	
//	// resolution will be scaled up to next power of 2
//	// max resolution is 32
//	HashSpace(int resolution);
//	~HashSpace();
//	
//	// hash an xyz position:
//	int unpackPos(float x, float y, float z) const;
//	template<typename T>
//	int unpackPos(Vec<3,T> v) const { return unpackPos(v[0], v[1], v[2]); }
//	
//	// the invalid position hash:
//	static int invalidPos();
//	// the invalid cell ID marker:
//	static int invalidCell();
//	
//	// get voxel index for given hashed position:
//	int cellIdx(int pos) const;
//	
//	// get x,y,z cell coordinate for a given hashed position:
//	void coords(int pos, int &x, int &y, int &z) const;
//	void cellCoords(int cpos, int &x, int &y, int &z) const;
//
//	// get number of objects:
//	int numObjects() const { return mObjects.size(); }
//	
//	// id is not bounds checked; it must be 0 <= id < numObjects()
//	Object& object(int id) { return mObjects[id]; }
//	
//	// query a location/radius to capture set of objects 
//	// nobjs is the max number of objects to return
//	// returns number of objects found
//	int query(float x, float y, float z, float radius, int nobjs, Object **res);
//	// exclude identifies an object to not include in the return
//	int query(float x, float y, float z, float radius, Object *exclude, int nobjs, Object **res);
//	// returns objects in a shell (r1 <= distance <= r2)
//	int queryRange(float x, float y, float z, float r1, float r2, int nobjs, Object **res);
//	
//	// set number of objects (resets all cells):
//	void rebuild(int nobjects);
//	
//	// id is not bounds checked; it must be 0 <= id < numObjects()
//	void hash(int id, float x, float y, float z);
//	template <typename T>
//	void hash(int id, Vec<3,T> pos) { hash(id, pos[0], pos[1], pos[2]); }
//	// hash a whole array of locations
//	// assumes 3-components, FloatTy, 2D Array
//	// index into Array corresponds to id
//	void hash(Array *points);
//	
//	// internal use:
//	void removeObject(Object * ob);
//
//	int mSize;	///< resolution of spatial hash
//	int mWrap;	///< size-1 for fast binary wrapping
//	
//	int mMaxd;	///< distance from origin to far corner
//	int mValidMask; ///< bitmask to detect valid range of positions
//	
//	std::vector<Cell> mCells;		// the voxels
//	std::vector<Object> mObjects;	// the objects
//	std::vector<int> mD;
//	std::vector<int> mG;
//};
//
//// Implementation --------------------------------------------------------------
//
//
//
//inline void HashSpace::Cell :: addObject(Object * ob) {
//	if(objects) {
//		// add to tail:
//		Object *pprev = objects->prev;
//		
//		objects->prev = ob;
//		ob->next = objects;
//		
//		pprev->next = ob;
//		ob->prev = pprev;
//	} else {
//	//	printf("\t 1 object: %d\n", cpos);
//		objects = ob;
//		ob->next = ob;
//		ob->prev = ob;
//	}
//}

} // al::

#endif
