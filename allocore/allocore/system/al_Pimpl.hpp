#ifndef INCLUDE_AL_PIMPL_HPP
#define INCLUDE_AL_PIMPL_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Helper class to reduce boiler-plate for Pimpl pattern

	File author(s):
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
