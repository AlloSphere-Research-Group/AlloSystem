#ifndef INCLUDE_AL_BUFFER_HPP
#define INCLUDE_AL_BUFFER_HPP

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
	Variably sized one-dimensional array

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <algorithm>
#include <vector>

namespace al{

/// Buffer

/// This buffer automatically expands itself as new elements are added.
/// Additionally, its logical size can be reduced without triggering memory
/// deallocations.
///
/// @ingroup allocore
template <class T, class Alloc=std::allocator<T> >
class Buffer : protected Alloc{
public:

	typedef T value_type;

	/// @param[in] size			Initial size
	explicit Buffer(int size=0)
	:	mElems(size), mSize(size)
	{}

	/// @param[in] size			Initial size
	/// @param[in] capacity		Initial capacity
	Buffer(int size, int capacity)
	:	mElems(capacity), mSize(size)
	{}

	~Buffer(){}


	int capacity() const { return mElems.size(); }		///< Returns total capacity
	int size() const { return mSize; }					///< Returns size
	const T * elems() const { return &mElems[0]; }		///< Returns C pointer to elements
	T * elems(){ return &mElems[0]; }					///< Returns C pointer to elements

	T * begin(){ return elems(); }
	const T * begin() const { return elems(); }
	T * end(){ return elems() + size(); }
	const T * end() const { return elems() + size(); }

	/// Get element at index
	T& operator[](int i){ return mElems[i]; }

	/// Get element at index (read-only)
	const T& operator[](int i) const { return mElems[i]; }

	/// Assign value to elements

	/// This function fills a Buffer with n copies of the given value. Note that
	/// the assignment completely changes the buffer and that the resulting size
	/// is the same as the number of elements assigned. Old data may be lost.
	void assign(int n, const T& v){ mElems.assign(n,v); }

	/// Get last element
	T& last(){ return mElems[size()-1]; }
	const T& last() const { return mElems[size()-1]; }

	/// Resets size to zero without deallocating allocated memory
	void reset(){ mSize=0; }

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
			Alloc::construct(elems()+size(), vsafecopy);
		}
		else{
			Alloc::construct(elems()+size(), v);
		}
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


	/// Insert new elements after each existing element

	/// @tparam n		Expansion factor; new size is n times old size
	/// @tparam dup		If true, new elements are duplicates of existing elements.
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
	std::vector<T, Alloc> mElems;
	int mSize;		// logical size array

	void setSize(int n){ mSize=n; }
};




/// Ring buffer

/// This buffer allows potentially large amounts of data to be buffered without
/// moving memory. This is accomplished by use of a moving write tap.
///
/// @ingroup allocore
template <class T, class Alloc=std::allocator<T> >
class RingBuffer : protected Alloc {
public:

	typedef T value_type;

	/// Default constructor; does not allocate memory
	RingBuffer(): mPos(-1){}

	/// @param[in] size		number of elements
	/// @param[in] v		value to initialize elements to
	explicit RingBuffer(unsigned size, const T& v=T())
	:	mPos(size)
	{
		resize(size,v);
	}


	/// Get number of elements
	int size() const { return mElems.size(); }

	/// Get absolute index of most recently written element
	int pos() const { return mPos; }

	/// Get fill amount of buffer
	int fill() const { return mFill; }


	/// Get element at absolute index
	T& operator[](int i){ return mElems[i]; }

	/// Get element at absolute index (read-only)
	const T& operator[](int i) const { return mElems[i]; }


	/// Obtain next element in buffer

	/// This method returns a reference to the next available element in the
	/// buffer. This is an alternative to calling write() that does not require
	/// constructing a new object, but instead returns the oldest element in
	/// the buffer. The returned reference should be assumed to be in an unknown
	/// state, thus should be initialized properly.
	T& next(){
		if(mFill < size()) ++mFill;
		++mPos; if(pos() == size()){ mPos=0; }
		return newest();
	}

	/// Write new element
	void write(const T& v){
		Alloc::construct(&next(), v);
	}

	/// Get reference to element relative to newest element
	const T& read(int i) const { return readFrom(pos(), i); }
	T& read(int i){ return const_cast<T&>(const_cast<const RingBuffer<T,Alloc>*>(this)->read(i)); }

	/// Get reference to older element relative to some newer element 

	/// @param[in] from		absolute index the read is relative to
	/// @param[in] dist		distance into past relative to 'from' of the returned element
	const T& readFrom(int from, int dist) const {
		return mElems[wrapOnce(from-dist, size())];
	}
	T& readFrom(int from, int dist){
		return const_cast<T&>(const_cast<const RingBuffer<T,Alloc>*>(this)->readFrom(from,dist));
	}

	/// \returns reference to newest (last in) element
	const T& newest() const { return mElems[pos()]; }
	T& newest(){ return const_cast<T&>(const_cast<const RingBuffer<T,Alloc>*>(this)->newest()); }

	/// \returns reference to oldest (first in) element
	const T& oldest() const { return read(fill()-1); }
	T& oldest(){ return const_cast<T&>(const_cast<const RingBuffer<T,Alloc>*>(this)->oldest()); }

	/// Set write position to start of array and zero fill amount
	void reset(){
		mPos = size()-1;
		mFill = 0;
	}

	/// Resize buffer

	/// @param[in] n	number of elements
	/// @param[in] v	initialization value of newly allocated elements
	void resize(int n, const T& v=T()){
		mElems.resize(n,v);
		if(mPos >=n) mPos = n-1;
	}

protected:
	std::vector<T, Alloc> mElems;
	int mPos;
	int mFill = 0;

	// Moves value one period closer to interval [0, max)
	static int wrapOnce(int v, int max){
		if(v <  0)   return v+max;
		if(v >= max) return v-max;
		return v;
	}
};



/// Constant size shift buffer

/// This is a first-in, first-out buffer with a constant number of elements.
/// Adding new elements to the buffer physically moves existing elements. The
/// advantage of moving memory like this is that elements stay logically ordered
/// making access faster and operating on the history easier.
///
/// @ingroup allocore
template <int N, class T>
class ShiftBuffer{
public:

	typedef T value_type;

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
