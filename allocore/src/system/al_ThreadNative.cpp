#include <stdio.h>
#include <algorithm> // for std::swap
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Thread.hpp"

#ifdef AL_WINDOWS
	#define USE_THREADEX
#else
	#define USE_PTHREAD
#endif

namespace al {

#ifdef USE_PTHREAD
#include <pthread.h>

struct Thread::Impl{
	Impl()
	:	mHandle(0)
	{ //printf("Thread::Impl(): %p\n", this);
		pthread_attr_init(&mAttr);

		// threads are not required to be joinable by default, so make it so
		pthread_attr_setdetachstate(&mAttr, PTHREAD_CREATE_JOINABLE);
	}

	~Impl(){ //printf("Thread::~Impl(): %p\n", this);
		pthread_attr_destroy(&mAttr);
	}

	bool start(std::function<void(void)>& func){
		if(mHandle) return false;
		struct F{
			static void * cFunc(void * user){
				(*((std::function<void(void)> *)user))();
				return NULL;
			}
		};
		return 0 == pthread_create(&mHandle, &mAttr, F::cFunc, &func);
	}

	bool join(double timeout){
		if(pthread_join(mHandle, NULL) == 0){
			mHandle = 0;
			return true;
		}
		return false;
	}

	void priority(int v){
		struct sched_param param;
		if(v >= 1 && v <= 99){
			param.sched_priority = v;
			//pthread_setschedparam(mHandle, SCHED_FIFO, &param);
			// FIFO and RR (round-robin) are for real-time scheduling
			pthread_attr_setschedpolicy(&mAttr, SCHED_FIFO);
			//pthread_attr_setschedpolicy(&mAttr, SCHED_RR);
			pthread_attr_setschedparam(&mAttr, &param);
		}
		else{
			param.sched_priority = 0;
			//pthread_setschedparam(mHandle, SCHED_OTHER, &param);
			pthread_attr_setschedpolicy(&mAttr, SCHED_OTHER);
			pthread_attr_setschedparam(&mAttr, &param);
		}
	}

//	bool cancel(){
//		return 0 == pthread_cancel(mHandle);
//	}
//
//	void testCancel(){
//		pthread_testcancel();
//	}

	void * handle(){ return &mHandle; }

	pthread_t mHandle;
	pthread_attr_t mAttr;
};


void * Thread::current(){
	// pthread_t pthread_self(void);
	static pthread_t r;
	r = pthread_self();
	return (void*)(&r);
}


#elif defined(USE_THREADEX)

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <process.h>

class Thread::Impl{
public:
	Impl(): mHandle(NULL){}

	bool start(std::function<void(void)>& func){
		if(NULL==mHandle){
			struct F{
				static unsigned _stdcall cFunc(void * user){
					(*((std::function<void(void)> *)user))();
					return 0;
				}
			};
			mHandle = (HANDLE)_beginthreadex(NULL, 0, F::cFunc, &func, 0, NULL);
		}
		return NULL!=mHandle;
	}

	bool join(double timeout){
		if(mHandle){
			DWORD waitms = timeout>=0. ? (DWORD)(timeout*1e3 + 0.5) : INFINITE;
			long retval = WaitForSingleObject(mHandle, waitms);
			if(retval == WAIT_OBJECT_0){
				CloseHandle(mHandle);
				mHandle = NULL;
				return true;
			}
		}
		return false;
	}

	// TODO: Threadx priority
	void priority(int v){
	}

//	bool cancel(){
//		TerminateThread((HANDLE)mHandle, 0);
//		return true;
//	}
//
//	void testCancel(){
//	}

	void * handle(){ return &mHandle; }

	HANDLE mHandle;
};

#endif



Thread::Thread()
:	mImpl(new Impl), mJoinOnDestroy(false)
{}

Thread::Thread(ThreadFunction& func)
:	mImpl(new Impl), mJoinOnDestroy(false)
{
	start(func);
}

Thread::Thread(void * (*cThreadFunc)(void * userData), void * userData)
:	mImpl(new Impl), mJoinOnDestroy(false)
{
	start(cThreadFunc, userData);
}

Thread::Thread(const Thread& other)
:	mImpl(new Impl), mFunc(other.mFunc), mJoinOnDestroy(other.mJoinOnDestroy)
{
}

Thread::~Thread(){
	if(mJoinOnDestroy) join();
	delete mImpl;
}


void swap(Thread& a, Thread& b){
	using std::swap;
	swap(a.mImpl, b.mImpl);
	swap(a.mFunc, b.mFunc);
	swap(a.mJoinOnDestroy, b.mJoinOnDestroy);
}

Thread& Thread::operator= (Thread other){
	swap(*this, other);
	return *this;
}

/*Thread& Thread::operator= (const Thread& other){
//printf("Thread::operator=\n");
	if(this != &other){
		//join();
		// delete Impl only if we are sure we are the owner
		if(mImpl != other.mImpl) delete mImpl;

		// always construct a new Impl
		mImpl = new Impl;

		mCFunc = other.mCFunc;
		mJoinOnDestroy = other.mJoinOnDestroy;
	}
	return *this;
}*/

Thread& Thread::priority(int v){
	mImpl->priority(v);
	return *this;
}

bool Thread::start(void * (*threadFunc)(void * userData), void * userData){
	// FIXME: this can cause crash!!! since pointers are copied???
	return start([&](){ threadFunc(userData); });
}

bool Thread::start(std::function<void (void)> func){
	mFunc = func;
	return mImpl->start(mFunc);
}

bool Thread::start(ThreadFunction& func){
	return start([&func](){ func(); });
}

bool Thread::join(double timeout){
	return mImpl->join(timeout);
}

void * Thread::nativeHandle(){
	return mImpl->handle();
}

} // al::
