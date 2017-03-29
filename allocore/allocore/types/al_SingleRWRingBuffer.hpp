#ifndef INCLUDE_AL_SINGLE_READER_WRITER_RING_BUFFER_HPP
#define INCLUDE_AL_SINGLE_READER_WRITER_RING_BUFFER_HPP

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
	Passing data between a pair of threads without locking

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <cstring>

#include "allocore/system/pstdint.h"

namespace al {

/** Lock free single-reader-single-writer ring buffer.
 * Can be used to stream data safely between two threads, one being
 * a reader, one a writer. There is no locking in this ring buffer,
 * so it is ideal to pass data to and from a high priority thread
 * like an audio thread.
 */

/// @ingroup allocore
class SingleRWRingBuffer {
public:

    /** Allocate ringbuffer.
        Actual size rounded up to next power of 2. */
	SingleRWRingBuffer(size_t sz=256);

	~SingleRWRingBuffer();

    /** The number of bytes available for writing.
	*/
	size_t writeSpace() const;


    /** The number of bytes available for reading.
	*/
	size_t readSpace() const;


    /** Copy sz bytes from src into the ringbuffer.
        Returns bytes actually copied.
	*/
	size_t write(const char * src, size_t sz);

    /** Read sz bytes of data from the ring buffer and advance the read pointer.
		Returns bytes actually copied
	*/
	size_t read(char * dst, size_t sz);

    /** Read data without advancing the read pointer
        Returns bytes actually copied
	*/
	size_t peek(char * dst, size_t sz);

    /** Clear any data in the ringbuffer
	*/
	void clear();

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

inline size_t SingleRWRingBuffer :: writeSpace() const {
	const size_t r = mRead;
	const size_t w = mWrite;
	if (r==w) return mWrap;
	return ((mSize + (r - w)) & mWrap) - 1;
}

inline size_t SingleRWRingBuffer :: readSpace() const {
	const size_t r = mRead;
	const size_t w = mWrite;
	return (mSize + (w - r)) & mWrap;
}

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

void SingleRWRingBuffer::clear()
{
    mRead = mWrite;
}


} // al::

#endif /* include guard */
