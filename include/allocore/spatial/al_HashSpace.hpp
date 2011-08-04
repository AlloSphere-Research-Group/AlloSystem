#ifndef INCLUDE_AL_HASHSPACE_HPP
#define INCLUDE_AL_HASHSPACE_HPP

#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Array.hpp"

#include <vector>

/*!
	HashSpace is a way to detect object collisions using a voxel grid
	The grid has a given resolution (no. voxel cells per side)
	
	
	
*/

namespace al {

class HashSpace {
public:

	// Wrapper for an arbitary object to be stored in the space
	class Object {
	public:
		typedef void (*datafree)(void *);
		
		Vec3f pos;
		int oidx;		// my index in HashSpace.mObjects
		int idx;		
		int cellidx;	// my index in HashSpace.mCells
		Object *next;
		Object *prev;
		void *data;
		datafree dfree;
		
		Object()
		:	oidx(0),
			idx(0),
			cellidx(HashSpace::invalidCell()),
			next(0), 
			prev(0),
			data(0),
			dfree(0)
		{}
		
		Object(const Object& cpy)
		:	oidx(0),
			idx(0),
			cellidx(HashSpace::invalidCell()),
			next(0), 
			prev(0),
			data(0),
			dfree(0)
		{}
		
		~Object() {
			if(dfree && data) dfree(data);
		}
	};
	
	// The voxel is a list of objects 
	struct Cell {
		Cell() 
		: objects(0) {}
		
		Cell(const Cell& cpy) 
		: objects(0) {}
		
		void addObject(Object * ob);
		
		Object * objects;
	};
	
	
	// resolution will be scaled up to next power of 2
	// max resolution is 32
	HashSpace(int resolution);
	~HashSpace();
	
	// hash an xyz position:
	int unpackPos(float x, float y, float z) const;
	template<typename T>
	int unpackPos(Vec<3,T> v) const { return unpackPos(v[0], v[1], v[2]); }
	
	// the invalid position hash:
	static int invalidPos();
	// the invalid cell ID marker:
	static int invalidCell();
	
	// get voxel index for given hashed position:
	int cellIdx(int pos) const;
	
	// get x,y,z cell coordinate for a given hashed position:
	void coords(int pos, int &x, int &y, int &z) const;
	void cellCoords(int cpos, int &x, int &y, int &z) const;

	// get number of objects:
	int numObjects() const { return mObjects.size(); }
	
	// id is not bounds checked; it must be 0 <= id < numObjects()
	Object& object(int id) { return mObjects[id]; }
	
	// query a location/radius to capture set of objects 
	// nobjs is the max number of objects to return
	// returns number of objects found
	int query(float x, float y, float z, float radius, int nobjs, Object **res);
	// exclude identifies an object to not include in the return
	int query(float x, float y, float z, float radius, Object *exclude, int nobjs, Object **res);
	// returns objects in a shell (r1 <= distance <= r2)
	int queryRange(float x, float y, float z, float r1, float r2, int nobjs, Object **res);
	
	// set number of objects (resets all cells):
	void rebuild(int nobjects);
	
	// id is not bounds checked; it must be 0 <= id < numObjects()
	void hash(int id, float x, float y, float z);
	template <typename T>
	void hash(int id, Vec<3,T> pos) { hash(id, pos[0], pos[1], pos[2]); }
	// hash a whole array of locations
	// assumes 3-components, FloatTy, 2D Array
	// index into Array corresponds to id
	void hash(Array *points);
	
	// internal use:
	void removeObject(Object * ob);

	int mSize;	///< resolution of spatial hash
	int mWrap;	///< size-1 for fast binary wrapping
	
	int mMaxd;	///< distance from origin to far corner
	int mValidMask; ///< bitmask to detect valid range of positions
	
	std::vector<Cell> mCells;		// the voxels
	std::vector<Object> mObjects;	// the objects
	std::vector<int> mD;
	std::vector<int> mG;
};

// Implementation --------------------------------------------------------------



inline void HashSpace::Cell :: addObject(Object * ob) {
	if(objects) {
		// add to tail:
		Object *pprev = objects->prev;
		
		objects->prev = ob;
		ob->next = objects;
		
		pprev->next = ob;
		ob->prev = pprev;
	} else {
	//	printf("\t 1 object: %d\n", cpos);
		objects = ob;
		ob->next = ob;
		ob->prev = ob;
	}
}

} // al::

#endif
