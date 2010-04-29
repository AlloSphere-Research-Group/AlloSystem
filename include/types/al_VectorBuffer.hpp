#ifndef INCLUDE_AL_VECTORBUFFER_HPP
#define INCLUDE_AL_VECTORBUFFER_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
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
*/

#include <vector>

namespace al {

template<typename T>
class VectorBuffer {
public:
	VectorBuffer()
	:	mSize(0)
	{}

	~VectorBuffer() {}

	/// Set element at absolute index
	T& operator[](int i){ return mBuffer[i]; }
	
	/// Get element at absolute index
	const T& operator[](int i) const { return mBuffer[i]; }

	void clear() {mSize = 0;}
	void append(const T &v) {
		if(mSize >= (int)mBuffer.size()) {
			mBuffer.reserve(2*(mSize ? mSize : 4));
		}
		mBuffer[mSize] = v;
		mSize++;
	}
	
	void extend() {
		if(mSize >= (int)mBuffer.size()) {
			mBuffer.reserve(2*(mSize ? mSize : 4));
		}
		mSize++;
	}
	T & top() { return mBuffer[mSize-1]; }

	int size() const {return mSize;}

	T * data() {return &(mBuffer.front());}

protected:
	int				mSize;
	std::vector<T>	mBuffer;
};

} // al::

#endif
