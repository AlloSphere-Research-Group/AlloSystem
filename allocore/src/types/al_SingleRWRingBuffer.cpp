
#include <cstdint> // uint32_t
#include <cstring> // memcpy
#include "allocore/types/al_SingleRWRingBuffer.hpp"
using namespace al;

uint32_t nextPo2(uint32_t v){
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >>16;
	return v+1;
}

SingleRWRingBuffer::SingleRWRingBuffer(size_t sz)
:	mSize(nextPo2(sz)),
	mWrap(mSize-1),
	mRead(0),
	mWrite(0)
{
	mData = new char[mSize];
}

SingleRWRingBuffer::~SingleRWRingBuffer(){
	delete[] mData;
}

size_t SingleRWRingBuffer::writeSpace() const {
	const auto r = mRead;
	const auto w = mWrite;
	if (r==w) return mWrap;
	return ((mSize + (r - w)) & mWrap) - 1;
}

size_t SingleRWRingBuffer::readSpace() const {
	const auto r = mRead;
	const auto w = mWrite;
	return (mSize + (w - r)) & mWrap;
}

size_t SingleRWRingBuffer::write(const char * src, size_t sz){
	auto space = writeSpace();
	sz = sz > space ? space : sz;
	if (sz == 0) return 0;

	auto w = mWrite;
	auto end = w + sz;

	if(end < mSize){
		memcpy(mData+w, src, sz);
	} else {
		auto split = mSize-w;
		end &= mWrap;
		memcpy(mData+w, src, split);
		memcpy(mData, src+split, end);
	}

	mWrite = end;
	return sz;
}

size_t SingleRWRingBuffer::read(char * dst, size_t sz){
	auto space = readSpace();
	sz = sz > space ? space : sz;
	if(sz == 0) return 0;

	auto r = mRead;
	auto end = r + sz;

	if(end < mSize){
		memcpy(dst, mData+r, sz);
	} else {
		auto split = mSize-r;
		end &= mWrap;
		memcpy(dst, mData+r, split);
		memcpy(dst+split, mData, end);
	}

	mRead = end;
	return sz;
}

size_t SingleRWRingBuffer::peek(char * dst, size_t sz){
	auto space = readSpace();
	sz = sz > space ? space : sz;
	if (sz == 0) return 0;

	auto r = mRead;
	auto end = r + sz;

	if(end < mSize){
		memcpy(dst, mData+r, sz);
	} else {
		auto split = mSize-r;
		end &= mWrap;
		memcpy(dst, mData+r, split);
		memcpy(dst+split, mData, end);
	}
	return sz;
}

