#ifndef INC_AL_PIMPL_HPP
#define INC_AL_PIMPL_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Helper class to reduce boiler-plate for Pimpl pattern

	Author(s):
	Lance Putnam, 2021
*/

#include <memory> // unique_ptr
#include <string>

namespace al {

/// @addtogroup allocore
/// @{

/// Pimpl (Pointer to implementation) helper class

/// Based on
/// Sutter (2011). "GotW #101: Compilation Firewalls, Part 2", retrieved from 
/// https://herbsutter.com/gotw/_101/.
template<typename T>
class Pimpl {
public:
	Pimpl()
	:	m(new T, &defaultDelete){}

	template<typename ...Args> Pimpl(Args&& ... args)
	:	m(new T(std::forward<Args>(args)...), &defaultDelete){}

	~Pimpl(){}

	const T* operator->() const { return m.get(); }
	T* operator->(){ return m.get(); }
	const T& operator*() const { return *m.get(); }
	T& operator*(){ return *m.get(); }

private:
    std::unique_ptr<T, void (*)(T*)> m;

	// Needed to prevent compiler error about missing deleter type for
	// containing class with undefined destructor.
	// See http://oliora.github.io/2015/12/29/pimpl-and-rule-of-zero.html
	static void defaultDelete(T * t){
		static_assert(sizeof(T) > 0, "cannot delete incomplete type");
        static_assert(!std::is_void<T>::value, "cannot delete incomplete type");
		delete t;
	}
};

/// @} // end allocore group

} //al::
#endif
