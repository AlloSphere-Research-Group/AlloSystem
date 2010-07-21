#include "../private/al_ImplAPR.h"

#include "system/al_ThreadAPR.hpp"

#include "apr-1/apr_general.h"
#include "apr-1/apr_thread_proc.h"

namespace al {

struct ThreadAPR::Impl : public ImplAPR {
	Impl(void * ud = NULL) : ImplAPR() {
		check_apr(apr_threadattr_create(&mThreadAttr, mPool));
	}
	
	bool start(ThreadFunction routine, void * ptr) {
		if (mThread) return false;	// can't start already started!
		mRoutine = routine;
		mUserdata = ptr;
		apr_status_t rv = apr_thread_create(&mThread, mThreadAttr, threadfunc, this, mPool);
		check_apr(rv);
		return rv == APR_SUCCESS;
	}
	
	bool wait() {
		apr_status_t rv = APR_SUCCESS;
		rv = check_apr(apr_thread_join(&rv, mThread));
		mThread = 0;
		return rv == APR_SUCCESS;
	}
	
	~Impl() {
		if (mThread) wait();
	}
	
	apr_thread_t * mThread;
    apr_threadattr_t * mThreadAttr;
	
	ThreadFunction mRoutine;
	void * mUserdata;
	
	static void * APR_THREAD_FUNC threadfunc(apr_thread_t *thread, void *data) {
		printf(".\n");
		ThreadAPR::Impl * impl = (ThreadAPR::Impl *)data;
		void * result = (impl->mRoutine)(impl->mUserdata);
		apr_thread_exit(thread, APR_SUCCESS);
		return result;
	}
};



ThreadAPR :: ThreadAPR() {
	mImpl = new Impl();
}

ThreadAPR :: ThreadAPR(ThreadFunction routine, void * ptr) {
	mImpl = new Impl();
	start(routine, ptr);
}

ThreadAPR :: ~ThreadAPR() {
	delete mImpl;
}

bool ThreadAPR :: start(ThreadFunction routine, void * ptr) {
	return mImpl->start(routine, ptr);
}

bool ThreadAPR :: wait() {
	return mImpl->wait();
}

} // al::
