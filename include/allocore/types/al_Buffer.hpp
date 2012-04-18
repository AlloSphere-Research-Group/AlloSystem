#ifndef INCLUDE_AL_BUFFER_HPP
#define INCLUDE_AL_BUFFER_HPP

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
	Variably sized one-dimensional array

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <algorithm>
#include <vector>

namespace al{

/// Buffer

///
///
template <class T, class Alloc=std::allocator<T> >
class Buffer : protected Alloc{
	typedef Alloc super;
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


	int capacity() const { return mElems.size(); }		///< Returns total capacity
	int fill() const { return mFill; }					///< Returns buffer fill amount
	int pos() const { return mPos; }					///< Returns write position
	int size() const { return mSize; }					///< Returns size
	const T * elems() const { return &mElems[0]; }		///< Returns C pointer to elements
	T * elems(){ return &mElems[0]; }					///< Returns C pointer to elements


	/// Set element at absolute index
	T& operator[](int i){ return mElems[i]; }
	
	/// Get element at absolute index
	const T& operator[](int i) const { return mElems[i]; }

	/// Assign value to elements

	/// This function fills a Buffer with n copies of the given value. Note that
	/// the assignment completely changes the buffer and that the resulting size
	/// is the same as the number of elements assigned. Old data may be lost.
	void assign(int n, const T& v){ mElems.assign(n,v); }

	/// Set element at absolute index
	T& atAbs(int i){ return mElems[i]; }
	
	/// Get element at absolute index
	const T& atAbs(int i) const { return mElems[i]; }

	/// Set element at relative index
	T& atRel(int i){ return mElems[wrapOnce(pos()-i, size())]; }
	
	/// Get element at relative index
	const T& atRel(int i) const { return mElems[wrapOnce(pos()-i, size())]; }

	/// Get last element
	const T& last() const { return mElems[pos()]; }
	T& last(){ return mElems[pos()]; }

	/// Resets size to zero without deallocating allocated memory
	void reset(){ mSize=mFill=0; mPos=-1; }

	/// Resize buffer
	
	/// This will set both the size and capacity of the buffer to the requested 
	/// size. If the number is smaller than the current size the buffer is 
	/// truncated, otherwise the buffer is extended and new elements are
	/// default-constructed.
	void resize(int n){
		mElems.resize(n);
		setSize(n);
	}
	
	/// Set size of buffer
	
	/// If the requested size is larger than the current capacity, then the 
	/// buffer will be resized.
	void size(int n){
		if(capacity() < n) resize(n);
		else setSize(n);
	}

	/// Appends element to end of buffer growing its size if necessary
	void append(const T& v, double growFactor=2){
	
		// Grow array if too small
		if(size() >= capacity()){
			// Copy argument since it may be an element in current memory range
			// which may become invalid after the resize.
			const T vsafecopy = v;
			mElems.resize((size() ? size() : 4)*growFactor);
			super::construct(elems()+size(), vsafecopy);
		}
		else{
			super::construct(elems()+size(), v);
		}
		mPos=size();
		++mSize;
	}
	/// synonym for append():
	void push_back(const T& v, double growFactor=2) { append(v, growFactor); }	

	/// Append elements of another Buffer
	
	/// Note: not safe to apply this to itself
	///
	void append(const Buffer<T>& src){
		append(src.elems(), src.size());
	}

	/// Append elements of an array
	void append(const T * src, int len){
		int oldsize = size();
		size(size() + len);
		std::copy(src, src + len, mElems.begin() + oldsize);
	}
	
	/// Repeat last element
	void repeatLast(){ append(last()); }


	/// Write new element to ring buffer
	void write(const T& v){
		++mPos; if(pos() == size()){ mPos=0; }
		Alloc::construct(elems()+pos(), v);
		if(fill() < size()) ++mFill; 
	}

	/// Add new elements after each existing element
	
	/// @param[in] n	Expansion factor
	/// @param[in] dup	If true, new elements are duplicates of existing elements.
	///					If false, new elements are default constructed.
	template <int n, bool dup>
	void expand(){
		size(size()*n);
		const int Nd = dup ? n : 1;
		for(int i=size()/n-1; i>=0; --i){
			const T& v = (*this)[i];
			for(int j=0; j<Nd; ++j) Alloc::construct(elems()+n*i+j, v);
		}
	}

private:
	int mSize;		// number of elements in array
	int	mFill;		// number of elements written to buffer (up to size())
	int mPos;		// index of most recently written element
	std::vector<T, Alloc> mElems;
	
	void setSize(int n){
		mSize=n;
		if(mFill> n) mFill = n;
		if(mPos >=n) mPos  = n-1;
	}
	
	// Moves value one period closer to interval [0, max)
	static int wrapOnce(int v, int max){
		if(v <  0)   return v+max;
		if(v >= max) return v-max;
		return v;
	}
};





/// Fixed size shift buffer
template <int N, class T>
class ShiftBuffer{
public:

	/// @param[in] v	Value to initialize all elements to
	ShiftBuffer(const T& v=T()){ assign(v); }

	/// Get number of elements
	static int size(){ return N; }

	/// Get pointer to elements (read-only)
	const T * elems() const { return &mElems[0]; }
	
	/// Get pointer to elements
	T * elems(){ return &mElems[0]; }

	/// Get reference to element at index
	T& operator[](int i){ return mElems[i];}
	
	/// Get reference to element at index (read-only)
	const T& operator[](int i) const { return mElems[i]; }


	/// Push new element onto buffer. Newest element is at index 0.
	void operator()(const T& v){
		for(int i=N-1; i>0; --i) mElems[i] = mElems[i-1];
		mElems[0]=v;
	}


	/// Set all elements to argument
	void assign(const T& v){ for(int i=0;i<N;++i) mElems[i]=v; }

	/// Zero bytes of all elements
	void zero(){ memset(mElems, 0, N * sizeof(T)); }

protected:
	T mElems[N];
};




} // al::

#endif
