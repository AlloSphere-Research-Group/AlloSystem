#ifndef INCLUDE_AL_SINGLE_READER_WRITER_RING_BUFFER_HPP
#define INCLUDE_AL_SINGLE_READER_WRITER_RING_BUFFER_HPP

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
	Passing data between a pair of threads without locking

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/system/pstdint.h"

namespace al {

class SingleRWRingBuffer {
public:

	/*
		Allocate ringbuffer. 
		Actual size rounded up to next power of 2.
	*/
	SingleRWRingBuffer(size_t sz=256);
	
	~SingleRWRingBuffer();

	/* 
		The number of bytes available for writing.  
	*/
	size_t writeSpace() const;
	
	
	/* 
		The number of bytes available for reading.  
	*/
	size_t readSpace() const;

	
	/* 
		Copy sz bytes into the ringbuffer
		Returns bytes actually copied
	*/
	size_t write(const char * src, size_t sz);
	
	/* 
		Read data and advance the read pointer
		Returns bytes actually copied
	*/
	size_t read(char * dst, size_t sz);
	
	/* 
		Read data without advancing the read pointer
		Returns bytes actually copied
	*/
	size_t peek(char * dst, size_t sz);

protected:
	
	size_t mSize, mWrap;
	size_t mRead, mWrite;
	char * mData;
};	




inline uint32_t next_power_of_two(uint32_t v){
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >>16;
	return v+1;
}

/*
	Allocate ringbuffer. 
	Actual size rounded up to next power of 2.
*/
inline SingleRWRingBuffer :: SingleRWRingBuffer(size_t sz) 
:	mSize(next_power_of_two(sz)),
	mWrap(mSize-1),
	mRead(0),
	mWrite(0)
{
	mData = new char[mSize];
}

inline SingleRWRingBuffer :: ~SingleRWRingBuffer() {
	delete[] mData;
}

/* 
	The number of bytes available for writing.  
*/
inline size_t SingleRWRingBuffer :: writeSpace() const {
	const size_t r = mRead;
	const size_t w = mWrite;
	if (r==w) return mWrap;
	return ((mSize + (r - w)) & mWrap) - 1;	
}


/* 
	The number of bytes available for reading.  
*/
inline size_t SingleRWRingBuffer :: readSpace() const {
	const size_t r = mRead;
	const size_t w = mWrite;
	return (mSize + (w - r)) & mWrap;
}


/* 
	Copy sz bytes into the ringbuffer
	Returns bytes actually copied
*/
inline size_t SingleRWRingBuffer :: write(const char * src, size_t sz) {
	size_t space = writeSpace();
	sz = sz > space ? space : sz; 
	if (sz == 0) return 0;
	
	size_t w = mWrite;
	size_t end = w + sz;
	
	if (end < mSize) {
		memcpy(mData+w, src, sz);
	} else {
		size_t split = mSize-w;
		end &= mWrap;
		memcpy(mData+w, src, split);
		memcpy(mData, src+split, end);
	}
	
	mWrite = end;
	return sz;
}

/* 
	Read data and advance the read pointer
	Returns bytes actually copied
*/
inline size_t SingleRWRingBuffer :: read(char * dst, size_t sz) {
	size_t space = readSpace();
	sz = sz > space ? space : sz; 
	if (sz == 0) return 0;
	
	size_t r = mRead;
	size_t end = r + sz;
	
	if (end < mSize) {
		memcpy(dst, mData+r, sz);
	} else {
		size_t split = mSize-r;
		end &= mWrap;
		memcpy(dst, mData+r, split);
		memcpy(dst+split, mData, end);
	}
	
	mRead = end;
	return sz;
}

/* 
	Read data without advancing the read pointer
	Returns bytes actually copied
*/
inline size_t SingleRWRingBuffer :: peek(char * dst, size_t sz) {
	size_t space = readSpace();
	sz = sz > space ? space : sz; 
	if (sz == 0) return 0;
	
	size_t r = mRead;
	size_t end = r + sz;
	
	if (end < mSize) {
		memcpy(dst, mData+r, sz);
	} else {
		size_t split = mSize-r;
		end &= mWrap;
		memcpy(dst, mData+r, split);
		memcpy(dst+split, mData, end);
	}
	return sz;
}

	
} // al::

#endif /* include guard */
