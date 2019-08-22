#include <stdio.h>
#include <algorithm> // for std::swap
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Thread.hpp"

#if !defined(AL_THREAD_USE_STD) && !defined(AL_THREAD_USE_THREADEX) && !defined(AL_THREAD_USE_PTHREAD)
	// Try to derive best thread backend to use
	#ifdef AL_WINDOWS
		#if defined(__MINGW32__) || defined(_MSC_VER)
			#define AL_THREAD_USE_THREADEX
		#else  // MinGW-w64 / MSYS2
			#define AL_THREAD_USE_STD
		#endif
	#else
		#define AL_THREAD_USE_PTHREAD
	#endif
#endif

using namespace al;

#ifdef AL_THREAD_USE_STD
#include <thread>

struct Thread::Impl{
	Impl(){}

	~Impl(){}

	bool start(Thread::Function& func){
		if(mThread.joinable()) return false; // invalid or already running
		mThread = std::thread(func);
		return mThread.joinable();
	}

	bool join(double timeout){
		if(mThread.joinable()){
			mThread.join();
			return true;
		}
		return false;
	}

	void priority(int v){
	}

	/*
	bool cancel(){
		return false;
	}

	void testCancel(){
		pthread_testcancel();
	}
	*/

	void * handle(){ return &mThread; }

	std::thread mThread;
};


void * Thread::current(){
	// pthread_t pthread_self(void);
	static pthread_t r;
	r = pthread_self();
	return (void*)(&r);
}

#elif defined(AL_THREAD_USE_PTHREAD)
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

	bool start(Thread::Function& func){
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
	
	/*
	bool cancel(){
		return 0 == pthread_cancel(mHandle);
	}

	void testCancel(){
		pthread_testcancel();
	}
	*/

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


#elif defined(AL_THREAD_USE_THREADEX)

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <process.h>

class Thread::Impl{
public:
	Impl(): mHandle(NULL){}

	bool start(Thread::Function& func){
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

	/*
	bool cancel(){
		TerminateThread((HANDLE)mHandle, 0);
		return true;
	}

	void testCancel(){
	}
	*/

	void * handle(){ return &mHandle; }

	HANDLE mHandle;
};

#endif



Thread::Thread()
:	mImpl(new Impl)
{}

Thread::Thread(Thread::Function func)
:	mImpl(new Impl)
{
	start(func);
}

Thread::Thread(const Thread& other)
:	mImpl(new Impl), mFunc(other.mFunc), mJoinOnDestroy(other.mJoinOnDestroy)
{
}

Thread::~Thread(){
	if(mJoinOnDestroy) join();
	delete mImpl;
}

Thread& Thread::operator= (Thread t){
	using std::swap;
	swap(mImpl, t.mImpl);
	swap(mFunc, t.mFunc);
	swap(mJoinOnDestroy, t.mJoinOnDestroy);
	return *this;
}

Thread& Thread::priority(int v){
	mImpl->priority(v);
	return *this;
}

bool Thread::start(Thread::Function func){
	mFunc = func;
	return mImpl->start(mFunc);
}

bool Thread::join(double timeout){
	return mImpl->join(timeout);
}

void * Thread::nativeHandle(){
	return mImpl->handle();
}

//} // al::
