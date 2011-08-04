#include "allocore/spatial/al_HashSpace.hpp"
#include "allocore/math/al_Functions.hpp"

#include <stdio.h>
#include <map>

// binary offsets per dim:
#define Bx 5
#define By Bx
#define Bz Bx

#define Tx ((1<<(Bx-1))-1)
#define Ty ((1<<(By-1))-1)
#define Tz ((1<<(Bz-1))-1)

#define Lx (1<<Bx)
#define Ly (1<<By)
#define Lz (1<<Bz)

#define Ox Bx
#define Oy (Ox+By)
#define Oz (Oy+Bz)
#define Oxu (Ox+Oz)
#define Oyu (Oy+Oz)

#define Sx 0
#define Sy Bx
#define Sz (Sy+By)
#define Syu (Sy+Oz)

#define Mx ((1<<Ox)-1)
#define My (((1<<Oy)-1)-Mx)
#define Mz (((1<<Oz)-1)-(My+Mx))
#define Myu (((1<<Oyu)-1)-((1<<Oxu)-1))
#define Mup (Mx + Mz + Myu)

#define InvalidCell (-1)
#define InvalidPos (0xFFFFFFFF)

#ifndef MAX
	#define MAX(a, b) ((a > b) ? (a) : (b))
#endif
#define ABS(a) ((a < 0) ? (-a) : (a))

#define NEGATIVE(v) ((*((int *)(&v))) & (0x80000000))

using std::vector;
using namespace al;

// convert integer location to binary representation:
int bpos(int x, int y, int z) {
	return ((x<<Sx)&Mx) + ((y<<Sy)&My) + ((z<<Sz)&Mz);
}
int bpos(Vec3i v) {
	return ((v[0]<<Sx)&Mx) + ((v[1]<<Sy)&My) + ((v[2]<<Sz)&Mz);
}

// convert bit location to integer coords:
void ccoords(int pos, int &x, int &y, int &z) {
	x = (pos & Mx) >> Sx;
	y = (pos & My) >> Sy;
	z = (pos & Mz) >> Sz;
	
	x = (x > Tx) ? (x-Lx) : x;
	y = (y > Ty) ? (y-Ly) : y;
	z = (z > Tz) ? (z-Lz) : z;
}

int unpack(int pos) {
	return (pos & (~My)) + ((pos & My) << Oz);
}

int pack(int pos) {
	pos = (pos & Mup);
	return ((pos & Myu) >> Oz) + (pos & ~Myu);
}

// integer distance squared
int dist2(int a1, int a2, int a3) {
	int x = MAX(0, ABS(a1)-1);
	int y = MAX(0, ABS(a2)-1);
	int z = MAX(0, ABS(a3)-1);
	return x*x+y*y+z*z;
}

void setA(vector< vector<int> > &A, int i, int j, int k) {
	int d2 = dist2(i, j, k);
	A[d2].push_back(
		unpack(bpos(i, j, k))
	);
}


void print_bin(float f) {
	int *x = (int *)(&f);
	char s[33];
	for(int i=31; i >= 0; i--) {
		s[31-i] = (*x)&(1<<i) ? '1' : '0';
	}
	s[32] = '\0';
	printf("%f\t%s\n", f, s);
}

void print_bini(int f) {
	int *x = (int *)(&f);
	char s[33];
	for(int i=31; i >= 0; i--) {
		s[31-i] = (*x)&(1<<i) ? '1' : '0';
	}
	s[32] = '\0';
	printf("%d\t%s\n", f, s);
}

HashSpace :: HashSpace(int resolution)
:	mSize(al::ceilPow2(resolution)),
	mWrap(mSize-1)
{	
	mMaxd = dist2(mWrap, mWrap, mWrap);
	mCells.resize(mSize*mSize*mSize);

	mValidMask = ~bpos(mWrap, mWrap, mWrap);
	
	std::vector< vector<int> > A(mMaxd+1);
	for(int i=0; i < mSize; i++) {
		for(int j=0; j < mSize; j++) {
			for(int k=0; k < mSize; k++) {
				setA(A, i, j, k);
				if(k != 0) setA(A, i, j, -k);
			}
			if(j != 0) {
				for(int k=0; k < mSize; k++) {
					setA(A, i, -j, k);
					if(k != 0) setA(A, i, -j, -k);
				}
			}
		}
		if(i != 0) {
			for(int j=0; j < mSize; j++) {
				for(int k=0; k < mSize; k++) {
					setA(A, -i, j, k);
					if(k != 0) setA(A, -i, j, -k);
				}
				if(j != 0) {
					for(int k=0; k < mSize; k++) {
						setA(A, -i, -j, k);
						if(k != 0) setA(A, -i, -j, -k);
					}
				}
			}
		}
	}
	// create lookup tables:
	mD.reserve(A.size());	
	for(unsigned i=0; i < A.size(); i++) {
		vector<int> &ilist = A[i];
		if(ilist.size() > 0) {
			for(unsigned j=0; j < ilist.size(); j++) {
				mG.push_back(ilist[j]);
			}
			mD[i] = mG.size();
		}
		else {
			mD[i] = mD[i-1];
		}
	}	
}

HashSpace :: ~HashSpace() {}

int HashSpace :: invalidPos() { return InvalidPos; }
int HashSpace :: invalidCell() { return InvalidCell; }

int HashSpace :: unpackPos(float x, float y, float z) const {
	int i = (int)x;
	int j = (int)y;
	int k = (int)z;
	
	int pos = bpos(i, j, k);
	if(pos & mValidMask || NEGATIVE(x) || NEGATIVE(y) || NEGATIVE(z)) {
		return InvalidPos;
	}
	else {
		return unpack(pos);
	}
}

int HashSpace :: cellIdx(int pos) const {
	return (pos & Mx) + (((pos & Myu) >> Syu) + ((pos & Mz) >> Sz)*mSize)*mSize;
}

void HashSpace :: coords(int pos, int &x, int &y, int &z) const {
	ccoords(pack(pos), x, y, z);
}

void HashSpace :: cellCoords(int cpos, int &x, int &y, int &z) const {
	int size2 = mSize*mSize;
	z = cpos/(size2);
	y = (cpos-z*size2)/mSize;
	x = cpos % mSize;
}

void HashSpace :: rebuild(int nobjects) {
	for(int i=0; i < mSize*mSize*mSize; i++) {
		mCells[i].objects = 0;
	}
	mObjects.resize(nobjects);
}


void debug_cell(HashSpace::Cell &cell) {
	if(cell.objects) {
		HashSpace::Object *fo = cell.objects;
		int i=0;
		do {
			printf("\t %d: %p obj: %p %p\n", i, fo, fo->prev, fo->next);
			fo = fo->next;
			i++;
		} while(fo != cell.objects);
	}
	else {
		printf("\t<no objects>\n");
	}
}

void HashSpace :: removeObject(Object * ob) {
	int pcellpos = ob->cellidx;
	// remove from cell:
	Cell &pcell = mCells[pcellpos];
	/*
	printf("pcell:\n");
	debug_cell(pcell);
	printf("\n");
	*/
	// cell only had 1 object:
	if(ob == ob->prev) {
		pcell.objects = 0;
	}
	else {
		Object *prev = ob->prev;
		Object *next = ob->next;
		prev->next = next;
		next->prev = prev;
		
		if(pcell.objects == ob) {
			pcell.objects = next;
		}
	}
	/*
	printf("pcell:\n");
	debug_cell(pcell);
	printf("\n");
	*/
}

void HashSpace :: hash(int id, float x, float y, float z) {
	Object * ob = &mObjects[id];
	ob->pos.set(x,y,z);
	int upos = unpackPos(x, y, z);
	if(upos == (int)InvalidPos) {
		if(ob->cellidx != InvalidCell) {
			removeObject(ob);	
					
			// mark object as removed:
			ob->cellidx = InvalidCell;
			ob->prev = 0;
			ob->next = 0;
		}
	} else {
		// the position in mCells
		int cellId = cellIdx(upos);
		int pcellpos = ob->cellidx;
		ob->oidx = id;
		ob->idx = upos;
		ob->cellidx = cellId;
		
		// validate cell:
		if(cellId < 0 || cellId > mSize*mSize*mSize) {
			printf("Error, bad cell: (%f %f %f) -> %d\n", x, y, z, cellId);
			ob->cellidx = InvalidCell;
		} else if(pcellpos != cellId) {
		//	printf("%d: change cell: %d %d\n", i, pcellpos, cpos);
			if(pcellpos != InvalidCell) {
				removeObject(ob);
			}
			
			// add object to cell:
			mCells[cellId].addObject(ob);
		}
	}
}


// assumes 3-components, FloatTy, 2D Array
void HashSpace :: hash(Array *points) {
	int npoints = points->header.dim[0]*points->header.dim[1];
	
	// resize object storage:
	if(numObjects() != npoints) rebuild(npoints);
	
	float *data = (float *)points->data.ptr;
	for(int i=0; i < npoints; i++) {
		hash(i, data[0], data[1], data[2]);
		data += 3;
	}
}

int HashSpace :: query(float x, float y, float z, float radius, int nobjs, Object **res) {
	int nres = 0;
	int upos = unpackPos(x, y, z);

	if(upos != (int)InvalidPos) {
		float d2 = radius*radius;
		int n = (int)d2;
		
		int maxidx = mD[al::min(n, mMaxd)];
		for(int i=0; i < maxidx; i++) {
			int offset = mG[i];
			int cellpos = offset+upos;
			int cidx = cellIdx(cellpos);
			
			printf("cellpos %d cidx %d cells %d\n", cellpos, cidx, mCells.size());
			
			if (cidx < mCells.size()) {
				Cell &cell = mCells[cidx];
				Object *o = cell.objects;
				if(o) {
					do {
						float dx = x - o->pos[0];
						float dy = y - o->pos[1];
						float dz = z - o->pos[2];
						float od2 = dx*dx+dy*dy+dz*dz;
						if(od2 <= d2) {
							res[nres] = o;
							nres++;
						}
						o = o->next;
					} while(o && o != cell.objects && nres < nobjs);
				}
			}
			
			if(nres == nobjs) {
				break;
			}
		}
	}
	return nres;
}


int HashSpace :: query(float x, float y, float z, float radius, Object *exclude, int nobjs, Object **res) {
	int nres = 0;
	int upos = unpackPos(x, y, z);
	if(upos != (int)InvalidPos) {
		float d2 = radius*radius;
		int n = (int)d2;
		
		int maxidx = mD[al::min(n, mMaxd)];
		for(int i=0; i < maxidx; i++) {
			int offset = mG[i];
			int cellpos = offset+upos;
			int cidx = cellIdx(cellpos);
			Cell &cell = mCells[cidx];
			if(cell.objects) {
				Object *o = cell.objects;
				do {
					if(o != exclude) {
						float dx = x - o->pos[0];
						float dy = y - o->pos[1];
						float dz = z - o->pos[2];
						float od2 = dx*dx+dy*dy+dz*dz;
						if(od2 <= d2) {
							res[nres] = o;
							nres++;
						}
					}
					o = o->next;
				} while(o != cell.objects && nres < nobjs);
			}
			
			if(nres == nobjs) {
				break;
			}
		}
	}
	return nres;
}

int HashSpace :: queryRange(float x, float y, float z, float r1, float r2, int nobjs, Object **res) {
	int nres = 0;
	int upos = unpackPos(x, y, z);
	if(upos != (int)InvalidPos) {
		float d12 = r1*r1;
		int n1 = (int)d12;
		
		float d22 = r2*r2;
		int n2 = (int)d22;
		
		int mind = al::min(n1-2, mMaxd);
		int minidx = mind < 0 ? 0 : mD[mind];
		int maxidx = mD[al::min(n2, mMaxd)];
		for(int i=minidx; i < maxidx; i++) {
			int offset = mG[i];
			int cellpos = offset+upos;
			int cidx = cellIdx(cellpos);
			Cell &cell = mCells[cidx];
			if(cell.objects) {
				Object *o = cell.objects;
				do {
					float dx = x - o->pos[0];
					float dy = y - o->pos[1];
					float dz = z - o->pos[2];
					float od2 = dx*dx+dy*dy+dz*dz;
					if(d12 <= od2 && od2 <= d22) {
						res[nres] = o;
						nres++;
					}
					o = o->next;
				} while(o != cell.objects && nres < nobjs);
			}
			
			if(nres == nobjs) {
				break;
			}
		}
	}
	return nres;
}


