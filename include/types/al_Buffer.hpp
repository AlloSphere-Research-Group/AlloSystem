#ifndef INCLUDE_AL_BUFFER_HPP
#define INCLUDE_AL_BUFFER_HPP

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

namespace al{

/// Buffer

///
///
template <class T, class Alloc=std::allocator<T> >
class Buffer : protected Alloc{
public:

	/// @param[in] size			Initial size
	explicit Buffer(int size=0)
	:	mSize(size), mFill(0), mPos(size-1), mElems(size)
	{}

	/// @param[in] size			Initial size
	/// @param[in] capacity		Initial capacity
	Buffer(int size, int capacity)
	:	mSize(size), mFill(0), mPos(size-1), mElems(capacity)
	{}

	~Buffer(){}


	int capacity() const { return mElems.capacity(); }	///< Returns total capacity
	int fill() const { return mFill; }					///< Returns buffer fill amount
	int pos() const { return mPos; }					///< Returns write position
	int size() const { return mSize; }					///< Returns size
	T * elems(){ return &mElems[0]; }					///< Returns C-style pointer to elements


	/// Set element at absolute index
	T& operator[](int i){ return atAbs(i); }
	
	/// Get element at absolute index
	const T& operator[](int i) const { return atAbs(i); }

	/// Set element at absolute index
	T& atAbs(int i){ return mElems[i]; }
	
	/// Get element at absolute index
	const T& atAbs(int i) const { return mElems[i]; }

	/// Set element at relative index
	T& atRel(int i){ return mElems[wrapOnce(pos()-i, size())]; }
	
	/// Get element at relative index
	const T& atRel(int i) const { return mElems[wrapOnce(pos()-i, size())]; }

//	/// Get last element
//	const T& last() const { return mElems[mSize-1]; }

	/// Clear
	void clear(){ mSize=mFill=0; mPos=-1; }

	/// Resize
	void resize(int n){
		mElems.resize(n);
		mSize=n;
		if(mFill >= n) mFill = n-1;
		if(mPos  >= n) mPos  = n-1;
	}

	/// Appends element to end of array doubling array size if necessary
	void append(const T &v){
		if(size() >= capacity()){	// double array size if too small
			mElems.resize((size() ? size() : 4)*2);
		}
		construct(elems()+size(), v);
		++mSize;
	}

	/// Write new element to ring buffer
	void write(const T& v){
		++mPos; if(pos() == size()){ mPos=0; }
		construct(elems()+pos(), v);
		if(fill() < size()) ++mFill; 
	}

private:
	int mSize;		// number of elements in array
	int	mFill;		// number of elements written to buffer (up to size())
	int mPos;		// circular buffer write position
	std::vector<T, Alloc> mElems;
	
	// Moves value one period closer to interval [0, max)
	static int wrapOnce(int v, int max){
		if(v <  0)   return v+max;
		if(v >= max) return v-max;
		return v;
	}
};

} // al::

#endif
