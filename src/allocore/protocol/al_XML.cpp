#include "allocore/protocol/al_XML.hpp"
#include "../private/al_ImplAPR.h"

#ifdef AL_LINUX
#include "apr-1.0/apr_general.h"
#include "apr-1.0/apr_xml.h"
#include "apr-1.0/apr_file_io.h"
#else
#include "apr-1/apr_general.h"
#include "apr-1/apr_xml.h"
#include "apr-1/apr_file_io.h"
#endif

namespace al {

struct XML::Impl : public ImplAPR {
	apr_xml_parser * parser;	
	apr_xml_doc * doc;
	
	Impl(const std::string src) : ImplAPR(), parser(NULL), doc(NULL) {
		parser = apr_xml_parser_create(mPool);
		check_apr(apr_xml_parser_feed(parser, src.c_str(), src.size()));
		check_apr(apr_xml_parser_done(parser, &doc));
		
		apr_xml_elem *root;
		root = doc->root;
		printf("root-element; name = %s, text = %s\n", root->name, root->first_cdata.first->text);
		//traverse_xml_tree(root, mp);
	}
	
	virtual ~Impl() {
		
	}
};

XML::XML(const std::string src)
:	mImpl(new Impl(src)) {

}

XML::~XML() {

}


///*
//struct Thread::Impl : public ImplAPR {
//	Impl(void * ud = NULL)
//	:	 ImplAPR(), mThread(0), mThreadAttr(0), mRoutine(0), mUserData(0)
//	{
//		check_apr(apr_threadattr_create(&mThreadAttr, mPool));
//	}
//
//	bool start(ThreadFunction routine, void * userData){
//		if(mThread) return false;	// can't start already started!
//		mRoutine = routine;
//		mUserData = userData;
//		apr_status_t rv = apr_thread_create(&mThread, mThreadAttr, cThreadFunc, this, mPool);
//		check_apr(rv);
//		return rv == APR_SUCCESS;
//	}
//
//	bool wait(){
//		apr_status_t rv = APR_SUCCESS;
//		rv = check_apr(apr_thread_join(&rv, mThread));
//		mThread = 0;
//		return rv == APR_SUCCESS;
//	}
//
//	~Impl(){
//		if(mThread) wait();
//	}
//
//	apr_thread_t * mThread;
//    apr_threadattr_t * mThreadAttr;
//	ThreadFunction mRoutine;
//	void * mUserData;
//
//	static void * APR_THREAD_FUNC cThreadFunc(apr_thread_t *thread, void *data){
//		//printf(".\n");
//		Impl * impl = (Impl *)data;
//		void * result = (impl->mRoutine)(impl->mUserData);
//		apr_thread_exit(thread, APR_SUCCESS);
//		return result;
//	}
//};
//*/
//
//
//struct Thread::Impl : public ImplAPR {
//	Impl()
//	:	 ImplAPR(), mThread(0), mThreadAttr(0), mFunc(0)
//	{
//		check_apr(apr_threadattr_create(&mThreadAttr, mPool));
//	}
//
//	bool start(ThreadFunction& func){
//		if(mThread) return false;	// can't start already started!
//		mFunc = &func;
//		apr_status_t rv = apr_thread_create(&mThread, mThreadAttr, cThreadFunc, this, mPool);
//		check_apr(rv);
//		return rv == APR_SUCCESS;
//	}
//
//	bool join(){
//		apr_status_t rv = APR_SUCCESS;
//		rv = check_apr(apr_thread_join(&rv, mThread));
//		mThread = 0;
//		return rv == APR_SUCCESS;
//	}
//
//	apr_thread_t * mThread;
//    apr_threadattr_t * mThreadAttr;
//	ThreadFunction * mFunc;
//	//void * mUserData;
//
//	static void * APR_THREAD_FUNC cThreadFunc(apr_thread_t *thread, void *data){
//		//printf(".\n");
//		Impl * impl = (Impl *)data;
//		(*impl->mFunc)();
//		apr_thread_exit(thread, APR_SUCCESS);
//		return NULL;
//	}
//};
//
//Thread::Thread()
//:	mImpl(new Impl()), mJoinOnDestroy(false)
//{}
//
//Thread::Thread(ThreadFunction& func)
//:	mImpl(new Impl()), mJoinOnDestroy(false)
//{
//	start(func);
//}
//
//Thread::Thread(void * (*cThreadFunc)(void * userData), void * userData)
//:	mImpl(new Impl()), mJoinOnDestroy(false)
//{
//	start(cThreadFunc, userData);
//}
//
//Thread::~Thread(){
//	if(mJoinOnDestroy) join();
//	delete mImpl;
//}
//
//bool Thread::start(ThreadFunction& func){
//	return mImpl->start(func);
//}
//
//bool Thread::join(){
//	return mImpl->join();
//}
//
////Thread::Thread()
////:	mImpl(new Impl())
////{}
////
////Thread::Thread(ThreadFunction routine, void * userData)
////:	mImpl(new Impl())
////{
////	start(routine, userData);
////}
////
////Thread::~Thread(){ delete mImpl; }
////
////bool Thread::start(ThreadFunction routine, void * userData){
////	return mImpl->start(routine, userData);
////}
////
////bool Thread::wait(){
////	return mImpl->wait();
////}

} // al::
