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
#include <cstddef>

namespace al{

/// Buffer

/// This buffer automatically expands itself as new elements are added.
/// Additionally, its logical size can be reduced without triggering memory
/// deallocations.
///
/// @ingroup allocore
template <class T>
class Buffer{
public:
	typedef std::size_t size_t;
	typedef T value_type;

	/// @param[in] size			Initial size
	explicit Buffer(size_t size=0)
	: Buffer(size,size) { }

	/// @param[in] size			Initial size
	/// @param[in] capacity		Initial capacity
	Buffer(size_t size, size_t capacity)
	{
		reserve(capacity);
		resize(size);
	}

	Buffer(const Buffer& other)
	{
		(*this) = other;
	}

	Buffer& operator=(const Buffer& other){
		if(this!=(&other)){
			resize(0);
			append(other);
		}
		return *this;
	}

	~Buffer(){ 
		delete[] mData;
		mData = nullptr;
		mEnd = nullptr;
		mCapEnd = nullptr;
	}

	size_t capacity() const { return mCapEnd-mData; }	///< Returns total capacity
	size_t size() const { return mEnd-mData; }					///< Returns size
	bool empty() const { return mData==mEnd; }			///< Whether buffer is empty
	const T * data() const { return mData; }		///< Returns C pointer to elements
	T * data(){ return mData; }					///< Returns C pointer to elements

	[[deprecated("elems is deprecated, use data instead")]]
	const T * elems() const { return mData; }		///< Returns C pointer to elements
	[[deprecated("elems is deprecated, use data instead")]]
	T * elems(){ return mData; }					///< Returns C pointer to elements

	T * begin(){ return mData; }
	const T * begin() const { return mData; }
	T * end(){ return mEnd; }
	const T * end() const { return mEnd; }

	/// Get element at index
	T& operator[](size_t i){ return mData[i]; }

	/// Get element at index (read-only)
	const T& operator[](size_t i) const { return mData[i]; }

	/// Assign value to elements

	/// This function fills a Buffer with n copies of the given value. Note that
	/// the assignment completely changes the buffer and that the resulting size
	/// is the same as the number of elements assigned. Old data may be lost.
	void assign(size_t n, const T& v){
		if(n > capacity()){
			auto oldData = mData;
			alloc(n);
			delete[] oldData;
		}
		mEnd = mData+n;
		for(auto& elem : *this){ new(&elem) value_type(v); }
	}

	/// Get last element
	T& last(){ return *(mEnd-1); }
	const T& last() const { return *(mEnd-1); }

	/// Resets size to zero without deallocating allocated memory
	void reset(){ resize(0); }

	/// This will set the capacity of the buffer to the requested size.
	/// If the number is smaller than the current capacity then nothing happens,
	/// otherwise the buffer is extended and new elements are default-constructed.
	void reserve(size_t n){
		if(n > capacity()){
			T* oldData = mData;
			T* oldEnd = mEnd;
			alloc(n);
			if(oldData){
				std::move(oldData, oldEnd, mData);
				delete[] oldData;
			}
		}
	}

	/// Resize buffer

	/// This will set both the size and capacity of the buffer to the requested
	/// size. If the number is smaller than the current size the buffer is
	/// truncated, otherwise the buffer is extended and new elements are
	/// default-constructed.
	void resize(size_t n){
		resize(n, value_type());
	}

	/// This will set both the size and capacity of the buffer to the requested
	/// size. If the number is smaller than the current size the buffer is
	/// truncated, otherwise the buffer is extended and new elements are
	/// copy-constructed from the provided val.
	void resize(size_t n, const value_type& val){
		if(size() < n){
			reserve(n);
			//When sizing up
			for (auto it = mEnd; it != mData+n; ++it)
			{
				new(it) value_type(val);
			}
		}
		else{
			//When sizing down
			for (auto it = mData+n; it != mEnd; ++it)
			{
				it->~value_type();
			}
		}
		mEnd = mData+n;
	}

	/// If the size of the buffer is smaller than its capacity then shrinks the
	/// capacity of the buffer to match the size.
	void shrink(){
		if(size()<capacity()){
			size_t shrinkTo = size() > 2 ? size() : 2;
			T* oldData = mData;
			T* oldEnd = mEnd;
			alloc(shrinkTo);
			if(oldData){
				std::move(oldData, oldEnd, mData);
				delete[] oldData;
			}
		}
	}

	/// Appends element to end of buffer growing its size if necessary
	void append(const T& v){
		// Grow array if too small
		if(mEnd == mCapEnd){
			// Copy argument since it may be an element in current memory range
			// which may become invalid after the resize.
			size_t newCapacity = capacity() > 0 ? capacity() * 2 : 2;
			reserve(newCapacity);
		}
		new(mEnd) value_type(v);
		mEnd++;
	}
	/// synonym for append():
	void push_back(const T& v) { append(v); }

	/// Append elements of another Buffer

	/// Note: not safe to apply this to itself
	///
	void append(const Buffer<T>& src){
		append(src.data(), src.size());
	}

	/// Append elements of an array
	void append(const T * src, size_t len){
		size_t newSize = size() + len;
		reserve(newSize);
		std::copy(src, src + len, end());
		mEnd += len;
	}

	/// Repeat last element
	void repeatLast(){ append(last()); }


	/// Insert new elements after each existing element

	/// @tparam n		Expansion factor; new size is n times old size
	/// @tparam dup		If true, new elements are duplicates of existing elements.
	///					If false, new elements are default constructed.
	template <size_t n, bool dup>
	void expand(){
		static_assert(n>1, "Invalid expansion factor");
		if(mData && size()>0){
			auto newSize = size()*n;
			if(newSize > capacity()){
				T* oldData = mData;
				alloc(newSize);
				for (size_t i = 0, iExp = 0; i < size(); i++, iExp+=n)
				{
					mData[iExp] = std::move(oldData[i]);
					for (size_t j = 1; j < n; j++)
					{
						if(dup){
							new(mData+(iExp +j)) value_type(mData[iExp]);
						}
						else{
							new(mData+(iExp +j)) value_type();
						}
					}
				}
				delete[] oldData;
			}
			else{
				for (size_t i = size()-1, iExp = newSize-1; i != static_cast<size_t>(-1); i--)
				{
					for (size_t k = 0; k < n; k++, iExp--)
					{
						if(k==n-1 || dup){
							new(mData+iExp) value_type(mData[i]);
						}
						else{
							new(mData+iExp) value_type();
						}
					}
				}
			}
			mEnd = mData+newSize;
		}
	}

private:
	T* mData = nullptr;
	T* mEnd = nullptr;
	T* mCapEnd = nullptr;
	void alloc(size_t newCapacity){
		if(newCapacity<=0){ newCapacity = 2; }
		auto tmpSize = size();
		mData = new T[newCapacity];
		mCapEnd = mData + newCapacity;
		if(tmpSize >= newCapacity){ mEnd = mCapEnd; }
		else{ mEnd = mData + tmpSize; }
	}
};

/// Ring buffer

/// This buffer allows potentially large amounts of data to be buffered without
/// moving memory. This is accomplished by use of a moving write tap.
///
/// @ingroup allocore
template <class T>
class RingBuffer {
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

	/// Get absolute index of oldest element
	int posOldest() const { auto i=mPos-(fill()-1); return i<0?i+size():i; }

	/// Get fill amount of buffer
	int fill() const { return mFill; }

	/// Returns whether buffer is full
	bool full() const { return size() == mFill; }

	/// Returns whether buffer is empty
	bool empty() const { return 0 == mFill; }

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

	/// Write (push) new element
	void write(const T& v){
		new(&next()) T(v);
	}

	/// Get reference to element relative to newest element
	const T& read(int i) const { return const_cast<RingBuffer*>(this)->read(i); }
	T& read(int i){ return readFrom(pos(), i); }

	/// Get reference to older element relative to some newer element 

	/// @param[in] from		absolute index the read is relative to
	/// @param[in] dist		distance into past relative to 'from' of the returned element
	const T& readFrom(int from, int dist) const {
		return const_cast<RingBuffer*>(this)->readFrom(from,dist);
	}
	T& readFrom(int from, int dist){
		return mElems[wrapOnce(from-dist, size())];
	}

	/// \returns reference to newest (last in) element
	const T& newest() const { return const_cast<RingBuffer*>(this)->newest(); }
	T& newest(){ return mElems[pos()]; }

	/// \returns reference to oldest (first in) element
	const T& oldest() const { return const_cast<RingBuffer*>(this)->oldest(); }
	T& oldest(){ return read(fill()-1); }

	/// Remove newest element from buffer
	void popNewest(){
		if(fill()){
			mPos = wrapOnce(mPos-1, size());
			--mFill;
		}
	}

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


	/// Iterator going from older to newer elements (write order)
	struct iterator {
		typedef T value_type;
		typedef T& reference;
		typedef T* pointer;
        typedef std::forward_iterator_tag iterator_category;
		typedef std::ptrdiff_t difference_type;

		iterator(T * data_, int len_, int tap_, int idx_)
			: data(data_), len(len_), tap(tap_), idx(idx_){}
		iterator& operator++(){ ++idx; return *this;}
		iterator operator++(int){ auto t=*this; ++(*this); return t;}
		bool operator==(iterator other) const { return idx == other.idx;}
		bool operator!=(iterator other) const { return !(*this == other);}
		reference operator*(){ auto i=idx+tap; return data[i<len?i:i-len]; }
		pointer operator->(){ return &*(*this); }
	private:
		T * data;
		int len, tap, idx;
	};

	/// @param[in] i	start index past oldest element
	iterator begin(int i=0){ return iterator(&mElems[0], mElems.size(), posOldest(), i); }
	iterator   end(){ return iterator(&mElems[0], mElems.size(), posOldest(), fill()); }

protected:
	Buffer<T> mElems;
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
