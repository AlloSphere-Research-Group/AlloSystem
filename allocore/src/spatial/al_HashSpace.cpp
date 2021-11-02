#include <cmath>
#include "allocore/spatial/al_HashSpace.hpp"

using namespace al;

void HashSpace::Voxel :: add(Object * o) {
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

void HashSpace::Voxel :: remove(Object * o) {
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



// of the matches, return the best:
HashSpace::Object * HashSpace::Query :: nearest(const HashSpace& space, const Object * src) {
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

// the maximum permissible value of radius is mDimHalf
// if int(inner^2) == int(outer^2), only 1 shell will be queried.
// TODO: non-toroidal version.
int HashSpace::Query :: operator()(const HashSpace& space, const HashSpace::Object * obj, double maxRadius, double minRadius) {
	unsigned nres = 0;
	const auto& center = obj->pos;
	const auto minr2 = minRadius*minRadius;
	const auto maxr2 = maxRadius*maxRadius;
	auto iminr2 = std::max<uint32_t>(0, minr2);
	auto imaxr2 = std::min<uint32_t>(space.mMaxHalfD2, 1. + (maxRadius+1.)*(maxRadius+1.));
	if (iminr2 < imaxr2) {
		auto cellstart = space.mDistanceToVoxelIndices[iminr2];
		auto cellend = space.mDistanceToVoxelIndices[imaxr2];
		for (auto i = cellstart; i < cellend; i++) {
			auto index = space.hash(center, space.mVoxelIndices[i]);
			const auto& voxel = space.mVoxels[index];
			// now add any objects in this voxel to the result...
			auto * head = voxel.mObjects;
			if (head) {
				auto * o = head;
				do {
					if (o != obj) {
						// final check - float version:
						auto rel = space.wrapRelative(o->pos - center);
						auto d2 = rel.magSqr();
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

int HashSpace::Query :: operator()(const HashSpace& space, Vec3d center, double maxRadius, double minRadius) {
	HashSpace::Object obj;
	obj.pos = center; // only pos used in following function
	return (*this)(space, &obj, maxRadius, minRadius);
}


// resolution can be 1 to 10; the dim is 2^resolution i.e. 2..1024
// (the limit is 10 so that the hash can fit inside a uint32_t integer)
// default 5 implies 32 units per side
HashSpace :: HashSpace(uint32_t resolution, uint32_t numObjects)
:	mShift(al::clip(resolution, uint32_t(10), uint32_t(1))),
	mShift2(mShift+mShift),
	mDim(1<<mShift),
	mDim2(mDim*mDim),
	mDim3(mDim2*mDim),
	mDimHalf(mDim/2),
	mWrap(mDim-1),
	mWrap3(mDim3-1)
{
	//printf("shift %d shift2 %d dim %d dim3 %d wrap %d wrap3 %d\n",
//		mShift, mShift2, mDim, mDim3, mWrap, mWrap3);
//
	mMaxD2 = distanceSquared(mWrap, mWrap, mWrap);

	// half-dim, because of toroidal wrapping
	mMaxHalfD2 = distanceSquared(mDimHalf, mDimHalf, mDimHalf);

	mVoxels.resize(mDim3);
	mVoxelIndicesToDistance.resize(mDim3);
	mObjects.resize(numObjects);
	for (unsigned i=0; i<mObjects.size(); i++) {
		mObjects[i].id = i;
	}

	// each voxel has a particular distance from the origin.
	// this can be used to create a reverse lookup table
	// where each distance shell contains a list of voxels
	// because of the tight packing,
	// this can be used to wrap on the entire voxel table
	// so e.g. a query can simply walk the lists...
	// it must handle +/- mDimHalf for a toroidal space
	std::vector<std::vector<uint32_t> > shells;
	shells.resize(mMaxHalfD2+1);
	mDistanceToVoxelIndices.resize(mMaxHalfD2+1);
	for(int x=-mDimHalf; x < mDimHalf; x++) {
		for(int y=-mDimHalf; y < mDimHalf; y++) {
			for(int z=-mDimHalf; z < mDimHalf; z++) {
				// each voxel lives at a given distance from the origin:
				//double d = distanceSquared(x+0.5, y+0.5, z+0.5);
				double d = distanceSquared(x, y, z);
				//double d = distanceSquared(x-0.5, y-0.5, z-0.5);
				//printf("%04d %04d %04d -> %8d\n", x, y, z, d);
				// if this is within the valid query radius:
				if (d < mMaxHalfD2) {
					// store the hash (voxel index) in the corresponding shell:
					//uint32_t h = hash(x+0.5, y+0.5, z+0.5);
					uint32_t h = hash(x, y, z);
					//uint32_t h = hash(x-0.5, y-0.5, z-0.5);
					shells[d].push_back(h);
				} else {
					//printf("out of range"); Vec3i(x, y, z).print();
				}

			}
		}
	}

	// now pack the shell indices into a sorted list
	// and store in a secondary list the offsets per distance
	for (unsigned d=0; d<mMaxHalfD2; d++) {
		std::vector<uint32_t>& shell = shells[d];
		if (!shell.empty()) {
			mDistanceToVoxelIndices[d] = mVoxelIndices.size();
			mVoxelIndicesToDistance[mVoxelIndices.size()] = d;
			for (unsigned j=0; j<shell.size(); j++) {
				mVoxelIndices.push_back(shell[j]);
			}
		} else {
			mDistanceToVoxelIndices[d] = d ? mDistanceToVoxelIndices[d-1] : 0;
		}
	}
	// store last shell:
	mDistanceToVoxelIndices[mMaxHalfD2] = mVoxelIndices.size();

//	// dump the lists:
//	uint32_t offset = hash(0, 1, 0);
//	printf("offset %d\n", offset);
//	Vec3i p = unhash(offset); p.print();
//	for (unsigned d=1; d<mDistanceToVoxelIndices.size(); d++) {
//		uint32_t cellstart = mDistanceToVoxelIndices[d-1];
//		uint32_t cellend = mDistanceToVoxelIndices[d];
//		printf("=== shell (squared distance) %d .. %d (%d .. %d) ===\n", d-1, d, cellstart, cellend);
//		for (uint32_t j=cellstart; j<cellend; j++) {
//			uint32_t voxel = unhash(offset + mVoxelIndices[j]) & mWrap3;
//			Vec3i p1 = unhash(voxel);
//			p1.print(); printf("\n");
//		}
//	}
}

HashSpace :: ~HashSpace() {}

void HashSpace :: numObjects(int numObjects) {
	mObjects.clear();
	mObjects.resize(numObjects);
	// clear all voxels:
	for (unsigned i=0; i<mVoxels.size(); i++) {
		mVoxels[i].mObjects = 0;
	}
}

HashSpace& HashSpace :: remove(uint32_t objectId) {
	Object& o = mObjects[objectId];
	if (o.hash != invalidHash()) mVoxels[o.hash].remove(&o);
	o.hash = invalidHash();
	return *this;
}

HashSpace& HashSpace :: move(uint32_t objectId, Vec3d pos) {
	Object& o = mObjects[objectId];
	o.pos = wrap(pos);
	uint32_t newhash = hash(o.pos);
	if (newhash != o.hash) {
		if (o.hash != invalidHash()) mVoxels[o.hash].remove(&o);
		o.hash = newhash;
		mVoxels[newhash].add(&o);
	}
	return *this;
}
