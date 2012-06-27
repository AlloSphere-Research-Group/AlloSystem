#ifndef INCLUDE_AL_MEMORY_HPP
#define INCLUDE_AL_MEMORY_HPP

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
	Arena / autorelease memory pool

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <string.h>

namespace al {

/*
	An Arena is a block from which to allocate items that will share a lifetime.
	When the lifetime is expired, there is no need to free the items allocated;
	just deleting the Arena (or letting it go out of scope if stack allocated) 
	will free all associated memory.
	
	AKA AutoReleasePool. 
	
	E.g.
	
	void test() {
		Arena arena;
		double * x = (double *)arena.calloc(sizeof(double) * 10);
		Foo * foo = arena.alloc<Foo>();
		Foo * foo1 = arena.New<Foo>();
		
		... // instructions using x and foo
		
	}	// stack variable arena goes out of scope, and frees memory of x and foo
*/
class Arena {
public:
	Arena();
	~Arena();
	
	void * alloc(size_t size) { return impl->alloc(size); }
	void * calloc(size_t size) { return impl->calloc(size); }
	
	template<typename T>
	T * alloc() { return alloc(sizeof(T)); }
	template<typename T>
	T * calloc() { return calloc(sizeof(T)); }
	
	template<typename T>
	T * New() { return new (alloc(sizeof(T))) T(); }
	
	
	class Impl {
	public:
		virtual ~Impl() {};
		virtual void * alloc(size_t size) = 0;
		virtual void * calloc(size_t size) { return memset(alloc(size), 0, size); };
	};
protected:
	Impl * impl;
};

} // al::

#endif /* include guard */
