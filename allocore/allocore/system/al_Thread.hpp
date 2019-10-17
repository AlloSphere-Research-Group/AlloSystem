#ifndef INCLUDE_AL_THREAD_HPP
#define INCLUDE_AL_THREAD_HPP

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
	Minimal thread class with similar interface to C++0x thread

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/


#include <functional> // std::function

namespace al{


/// Thread
///
/// @ingroup allocore
class Thread{
public:

	typedef std::function<void (void)> Function;


	/// Create thread without starting
	Thread();

	/// Start thread calling passed in function
	Thread(Function func);

	/// Copy constructor
	Thread(const Thread& other);

	~Thread();


	/// Set whether thread will automatically join upon destruction
	Thread& joinOnDestroy(bool v){ mJoinOnDestroy=v; return *this; }

	/// Set thread priority

	/// @param[in] v	priority of thread in [0, 99]. A value greater than 0
	///					makes the thread "real-time".
	Thread& priority(int v);


	/// Start executing function
	bool start(Function func);


	/// Block the calling routine indefinitely until the thread terminates

	///	This function suspends execution of the calling routine until the thread has
	///	terminated.  It will return immediately if the thread was already
	///	terminated.  A \e true return value signifies successful termination.
	///	A \e false return value indicates a problem with the wait call.
	bool join(double timeoutSec=-1);

	/// Returns pointer to implementation-specific thread handle
	void * nativeHandle();

	/// Return pointer to current OS thread object

	/// E.g., if using pthreads internally, will return the pthread_t.
	/// Will return NULL if current thread is not available.
	static void * current();

	Thread& operator= (Thread other);

protected:
	class Impl;
	Impl * mImpl;
	Function mFunc;
	bool mJoinOnDestroy = false;
};



/// Multiple threads acting as a single work unit
///
/// @ingroup allocore
template <class ThreadFunction>
class Threads{
public:

	/// A thread and function
	struct Worker{
		Thread thread;
		ThreadFunction function;
	};


	/// @param[in] size		number of worker threads
	Threads(int size = 0)
	:	mSize(0), mWorkers(0)
	{
		resize(size);
	}

	~Threads(){ clear(); }

	/// Returns number of workers
	int size() const { return mSize; }

	/// Resize number of workers
	void resize(int n){
		if(n != size()){
			mSize = n;
			clear();
			mWorkers = new Worker[size()];
		}
	}

	/// Start all worker threads
	void start(bool joinAll=true){
		for(int i=0; i<size(); ++i){
			thread(i).start(std::ref(function(i)));
		}
		if(joinAll) join();
	}

	/// Join all worker threads
	void join(){
		//*
		int joined=0;
		int i=0;
		while(joined < size()){
			//printf("trying to join %d\n", i);
			if(thread(i).join(0.)){
				//printf("\tjoined %d\n", i);
				++joined;
			}
			(++i) %= size();
		}//*/
		//for(int i=0; i<size(); ++i) thread(i).join(0.);
	}

	/// Get a worker
	Worker& worker(int i){ return mWorkers[i]; }

	/// Get a worker thread
	Thread& thread(int i){ return worker(i).thread; }

	/// Get a worker thread function
	ThreadFunction& function(int i){ return worker(i).function; }

	/// Get worker sub-interval range of a full interval [min, max)

	/// This is useful for determining how to break up for loops into
	/// sub-intervals. E.g., if the full loop interval is [ 0, N ), then the
	/// ith worker's interval is [ range(N)*i, range(N)*(i+1) ).
	template <class T>
	double range(T max, T min=T(0)){
		return (max-min) / double(size());
	}

	/// Get worker sub-interval of a full interval [min, max)

	/// @param[out] interval	two-element array containing endpoints of sub-interval
	/// @param[in]  i			worker index
	/// @param[in]  max			full interval max endpoint
	/// @param[in]  min			full interval min endpoint
	template <class T>
	void getInterval(T * interval, int i, T max, T min = T(0)){
		double diam = range(max,min);
		double left = diam * i + min;
		interval[0] = left;
		interval[1] = left + diam;
	}

protected:
	int mSize;
	Worker * mWorkers;

	void clear(){ if(mWorkers) delete[] mWorkers; }
};

} // al::

#endif

