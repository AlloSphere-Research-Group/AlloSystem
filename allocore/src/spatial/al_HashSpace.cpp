#include "allocore/spatial/al_HashSpace.hpp"
#include "allocore/math/al_Functions.hpp"

using namespace al;

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

