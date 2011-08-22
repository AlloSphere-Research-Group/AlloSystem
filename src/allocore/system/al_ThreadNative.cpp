#include "allocore/system/al_Thread.hpp"

#define USE_PTHREAD		(defined (__APPLE__) || defined (OSX) || defined (__LINUX__) || defined (__UNIX__))
#define USE_THREADEX	(defined(WIN32))

namespace al {

#if USE_PTHREAD
#include <pthread.h>

//typedef pthread_t ThreadHandle;
//typedef void * (*ThreadFunction)(void *);
//#define THREAD_FUNCTION(name) void * name(void * user)

struct Thread::Impl{
	Impl(): mHandle(0){}

	bool start(ThreadFunction& func){
		if(mHandle) return false;
		return 0 == pthread_create(&mHandle, NULL, cThreadFunc, &func);
	}

	bool join(){
		if(pthread_join(mHandle, NULL) == 0){
			mHandle = 0;
			return true;
		}
		return false;
	}

//	bool cancel(){
//		return 0 == pthread_cancel(mHandle);
//	}
//
//	void testCancel(){
//		pthread_testcancel();
//	}

	pthread_t mHandle;

	static void * cThreadFunc(void * user){
		ThreadFunction& tfunc = *((ThreadFunction*)user);
		tfunc();
		return NULL;
	}
};

#elif USE_THREADEX

#include <windows.h>
#include <process.h>

//typedef unsigned long ThreadHandle;
//typedef unsigned (__stdcall *ThreadFunction)(void *);
//#define THREAD_FUNCTION(name) unsigned _stdcall * name(void * user)

struct Thread::Impl{
	Impl(): mHandle(0){}

	bool start(ThreadFunction& func){
		if(mHandle) return false;
		unsigned thread_id;
		mHandle = _beginthreadex(NULL, 0, cThreadFunc, &func, 0, &thread_id);
		if(mHandle) return true;
		return false;
	}

	bool join(){
		long retval = WaitForSingleObject((HANDLE)mHandle, INFINITE);
		if(retval == WAIT_OBJECT_0){
			CloseHandle((HANDLE)mHandle);
			mHandle = 0;
			return true;
		}
		return false;
	}

//	bool cancel(){
//		TerminateThread((HANDLE)mHandle, 0);
//		return true;
//	}
//
//	void testCancel(){
//	}

	unsigned long mHandle;
//	ThreadFunction mRoutine;

	static unsigned _stdcall * cThreadFunc(void * user){
		ThreadFunction& tfunc = *((ThreadFunction*)user);
		tfunc();
		return NULL;
	}
};

#endif



Thread::Thread()
:	mImpl(new Impl()), mJoinOnDestroy(false)
{}

Thread::Thread(ThreadFunction& func)
:	mImpl(new Impl()), mJoinOnDestroy(false)
{
	start(func);
}

Thread::Thread(void * (*cThreadFunc)(void * userData), void * userData)
:	mImpl(new Impl()), mJoinOnDestroy(false)
{
	start(cThreadFunc, userData);
}

Thread::~Thread(){
	if(mJoinOnDestroy) join();
	delete mImpl;
}

bool Thread::start(ThreadFunction& func){
	return mImpl->start(func);
}

bool Thread::join(){
	return mImpl->join();
}

} // al::
