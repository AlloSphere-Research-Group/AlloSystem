#ifndef INC_AL_THREAD_HPP
#define INC_AL_THREAD_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Minimal thread class with similar interface to C++0x thread

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <functional> // std::function

namespace al{

/// @addtogroup allocore
/// @{

/// Thread
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

/// @} // end allocore group

} // al::
#endif
