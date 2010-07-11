#include "../private/al_ImplAPR.h"

#include "system/al_ThreadAPR.hpp"

#include "apr_general.h"
#include "apr_thread_proc.h"

using namespace al;



struct ThreadAPR::Impl : public ImplAPR {
	Impl(void * ud = NULL) : ImplAPR() {
		check_apr(apr_threadattr_create(&mThreadAttr, mPool));
	}
	
	bool start(ThreadFunction routine, void * ptr) {
		mRoutine = routine;
		mUserdata = ptr;
		apr_status_t rv = apr_thread_create(&mThread, mThreadAttr, threadfunc, ptr, mPool);
		check_apr(rv);
		return rv == APR_SUCCESS;
	}
	
	bool cancel() {
		apr_status_t rv = APR_SUCCESS;
		rv = check_apr(apr_thread_exit(mThread, rv));
		return rv == APR_SUCCESS;
	}
	
	bool wait() {
		apr_status_t rv = APR_SUCCESS;
		rv = check_apr(apr_thread_join(&rv, mThread));
		return rv == APR_SUCCESS;
	}
	
	~Impl() {
		cancel();
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

bool ThreadAPR :: cancel() {
	return mImpl->cancel();
}

bool ThreadAPR :: wait() {
	return mImpl->wait();
}

