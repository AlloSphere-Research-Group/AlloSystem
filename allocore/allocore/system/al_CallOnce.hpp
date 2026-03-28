#ifndef INC_AL_CALL_ONCE_H
#define INC_AL_CALL_ONCE_H

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Utility that will call block of code only once

	Author(s):
	Lance Putnam, 2021, putnam.lance@gmail.com
*/

namespace al{

namespace detail{
struct CallOnce{ template <class F> CallOnce(const F& f){ f(); } };
}

/// Calls the passed in code once on first pass

/// Useful for initializing static variables without any overhead of
/// conditionals.
#define AL_CALL_ONCE(...){\
	static al::detail::CallOnce callOnce([&](){__VA_ARGS__});\
}

} // al::
#endif
