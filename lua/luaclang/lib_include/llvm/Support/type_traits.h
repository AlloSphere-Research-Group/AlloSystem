//===- llvm/Support/type_traits.h - Simplfied type traits -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a template class that determines if a type is a class or
// not. The basic mechanism, based on using the pointer to member function of
// a zero argument to a function was "boosted" from the boost type_traits
// library. See http://www.boost.org/ for all the gory details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_TYPE_TRAITS_H
#define LLVM_SUPPORT_TYPE_TRAITS_H

// This is actually the conforming implementation which works with abstract
// classes.  However, enough compilers have trouble with it that most will use
// the one in boost/type_traits/object_traits.hpp. This implementation actually
// works with VC7.0, but other interactions seem to fail when we use it.

namespace llvm {

namespace dont_use
{
    // These two functions should never be used. They are helpers to
    // the is_class template below. They cannot be located inside
    // is_class because doing so causes at least GCC to think that
    // the value of the "value" enumerator is not constant. Placing
    // them out here (for some strange reason) allows the sizeof
    // operator against them to magically be constant. This is
    // important to make the is_class<T>::value idiom zero cost. it
    // evaluates to a constant 1 or 0 depending on whether the
    // parameter T is a class or not (respectively).
    template<typename T> char is_class_helper(void(T::*)());
    template<typename T> double is_class_helper(...);
}

template <typename T>
struct is_class
{
  // is_class<> metafunction due to Paul Mensonides (leavings@attbi.com). For
  // more details:
  // http://groups.google.com/groups?hl=en&selm=000001c1cc83%24e154d5e0%247772e50c%40c161550a&rnum=1
 public:
    enum { value = sizeof(char) == sizeof(dont_use::is_class_helper<T>(0)) };
};

  
// enable_if_c - Enable/disable a template based on a metafunction
template<bool Cond, typename T = void>
struct enable_if_c {
  typedef T type;
};

template<typename T> struct enable_if_c<false, T> { };
  
// enable_if - Enable/disable a template based on a metafunction
template<typename Cond, typename T = void>
struct enable_if : public enable_if_c<Cond::value, T> { };

namespace dont_use {
  template<typename Base> char base_of_helper(const volatile Base*);
  template<typename Base> double base_of_helper(...);
}

/// is_base_of - Metafunction to determine whether one type is a base class of
/// (or identical to) another type.
template<typename Base, typename Derived>
struct is_base_of {
  static const bool value 
    = is_class<Base>::value && is_class<Derived>::value &&
      sizeof(char) == sizeof(dont_use::base_of_helper<Base>((Derived*)0));
};

}

#endif
