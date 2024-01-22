#ifndef INCLUDE_AL_VEC_HPP
#define INCLUDE_AL_VEC_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Generic fixed-size n-vector

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/


#include <cmath> // abs, pow
#include <cstdio>
#include <initializer_list>
#include <ostream>
#include <type_traits> //remove_reference, is_polymorphic

namespace al {

template <int N, class T> class Vec;

typedef Vec<2,float>	Vec2f;	///< float 2-vector
typedef Vec<2,double>	Vec2d;	///< double 2-vector
typedef Vec<2,int>		Vec2i;	///< integer 2-vector
typedef Vec<3,float>	Vec3f;	///< float 3-vector
typedef Vec<3,double>	Vec3d;	///< double 3-vector
typedef Vec<3,int>		Vec3i;	///< integer 3-vector
typedef Vec<4,float>	Vec4f;	///< float 4-vector
typedef Vec<4,double>	Vec4d;	///< double 4-vector
typedef Vec<4,int>		Vec4i;	///< integer 4-vector


// Forward iterates from 0 to n-1. Current index is 'i'.
#define IT(n) for(int i=0; i<(n); ++i)

template <int N, class T>
struct VecElems{ T x,y,z,w; private: T data[N-4]; };

template<class T> struct VecElems<0,T>{};
template<class T> struct VecElems<1,T>{ T x; };
template<class T> struct VecElems<2,T>{ T x,y; };
template<class T> struct VecElems<3,T>{
	T x,y,z;

	/// Returns cross product of this x b
	Vec<3,T> cross(const Vec<3,T>& b) const {
		return Vec<3,T>( y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x );
	}
};
template<class T> struct VecElems<4,T>{ T x,y,z,w; };

/// Flag type to prevent element initialization
static struct VecNoInit{} VEC_NO_INIT;

// Base case
template <class V>
Vec<1,V> toVec(const V& v);

/// Returns new Vec filled with values
template <class V, class... Vs>
Vec<1+sizeof...(Vs),V> toVec(const V& v, Vs... vs);


/// Fixed-size n-vector

/// This is a fixed size array to enable better loop unrolling optimizations
/// by the compiler and to avoid an extra 'size' data member for small-sized
/// arrays.
///
/// @ingroup allocore
template <int N, class T>
class Vec : public VecElems<N,T>{

	static constexpr int const_abs(int x){ return x>=0 ? x : -x; }
	static constexpr int const_min(int x, int y){ return x<=y ? x : y; }
	static constexpr int const_mods(int x, int y){ return (x%y + y) % y; }

public:

	typedef T value_type;


	/// @param[in] v		value to initialize all elements to
	Vec(const T& v=T()){ *this = v; }

	/// @param[in] v1		value to initialize first element
	/// @param[in] v2		value to initialize second element
	Vec(const T& v1, const T& v2){ set(v1, v2); }

	/// @param[in] v1		value to initialize first element
	/// @param[in] v2		value to initialize second element
	/// @param[in] v3		value to initialize third element
	Vec(const T& v1, const T& v2, const T& v3){ set(v1, v2, v3); }

	/// @param[in] v1		value to initialize first element
	/// @param[in] v2		value to initialize second element
	/// @param[in] v3		value to initialize third element
	/// @param[in] v4		value to initialize fourth element
	Vec(const T& v1, const T& v2, const T& v3, const T& v4){ set(v1, v2, v3, v4); }

	/// @param[in] v		values to initialize elements to
	/// If the initializer list has one element, then it is assigned to all
	/// vector components.
	Vec(std::initializer_list<T> v){ set(v); }

	/// @param[in] v		vector to initialize elements to
	template <int N2, class T2>
	Vec(const Vec<N2, T2>& v){ *this = v; initTail<N2>(); }

	/// @param[in] v		vector to initialize first N-1 elements to
	/// @param[in] s		value of last element
	template <class Tv, class Ts>
	Vec(const Vec<N-1, Tv>& v, const Ts& s){ set(v,s); }

	/// @param[in] s		value of first element
	/// @param[in] v		vector to initialize last N-1 elements to
	template <class Ts, class Tv>
	Vec(const Ts& s, const Vec<N-1, Tv>& v){ set(s,v); }

	/// @param[in] v		pointer to array to initialize elements to
	/// @param[in] stride	stride factor through array
	template <class T2>
	Vec(const T2 * v, int stride=1){ set(v,stride); }

	/// Non-initializing constructor
	Vec(VecNoInit){}


	//--------------------------------------------------------------------------
	// Factory Methods

	/// Get axis aligned vector
	static Vec aa(int axis, T val = T(1)){
		return Vec().setAA(axis, val);
	}
	template <int Axis>
	static Vec aa(T val = T(1)){
		return Vec().setAA<Axis>(val);
	}

	/// Get vector filled with linear sequence of values

	/// @param[in] begin	start value
	/// @param[in] inc		difference between successive elements (slope)
	static Vec iota(T begin = T(0), T inc = T(1)){
		Vec r;
		auto val = begin;
		for(auto& v : r){ v = val; val+=inc; }
		return r;
	}

	/// Get vector filled with linear ramp

	/// @param[in] begin	start value
	/// @param[in] end		end value
	/// @tparam endInc		whether end value is inclusive
	template <bool endInc = true>
	static Vec line(T begin = T(0), T end = T(1)){
		constexpr auto M = N - int(endInc);
		static_assert(M>0, "Invalid number of steps");
		constexpr auto m = 1./M;
		return iota(begin, (end-begin)*m);
	}


	//--------------------------------------------------------------------------
	// Memory Operations

	/// Returns number of elements
	static constexpr int size(){ return N; }

	/// Returns C array type punned into a vector
	static Vec& pun(T * src){ return *(Vec*)(src); }
	static const Vec& pun(const T * src){ return *(const Vec*)(src); }

	/// Get reference to self as another type
	template <class V>
	V& as(){
		static_assert(sizeof(V) <= sizeof(*this), "Attempt to pun vector to object of larger size");
		return *(V *)(elems());
	}

	template <class V>
	const V& as() const {
		return const_cast<Vec*>(this)->as<V>();
	}

	/// Get vector with elements casted to new type
	template <class V>
	Vec<N,V> to() const { return Vec<N,V>(*this); }

	/// Get read-only pointer to elements
	const T * elems() const { return const_cast<Vec*>(this)->elems(); }

	/// Get read-write pointer to elements
	T * elems(){ return (T*)(this); }

	T * begin(){ return elems(); }
	const T * begin() const { return elems(); }
	T * end(){ return elems() + N; }
	const T * end() const { return elems() + N; }

	/// Set element at index with no bounds checking
	T& operator[](int i){ return elems()[i];}

	/// Get element at index with no bounds checking
	const T& operator[](int i) const { return elems()[i]; }

	T& at(int i){ return (*this)[i]; }
	const T& at(int i) const { return (*this)[i]; }

	/// Set element at index with compile-time bounds checking
	template <int i>
	T& at(){
		static_assert(0<=i && i<N, "Index out of bounds");
		return at(i);
	}

	/// Get element at index with compile-time bounds checking
	template <int i>
	const T& at() const {
		return const_cast<Vec*>(this)->at<i>();
	}

	/// Access first element
	T& front(){ return at<0>(); }
	const T& front() const { return const_cast<Vec*>(this)->front(); }

	/// Access last element
	T& back(){ return at<N-1>(); }
	const T& back() const { return const_cast<Vec*>(this)->back(); }

	Vec& operator = (const T& v){ return fill(v); }

	template <int N2, class T2>
	Vec& operator = (const Vec<N2,T2>& v){ IT(N<N2?N:N2) at(i) = T(v[i]); return *this; }

	/// Set element at index (a chainable version of at())
	template <int i>
	Vec& set(const T& v){ at<i>()=v; return *this; }

	/// Set elements from another vector and scalar
	template <class Tv, class Ts>
	Vec& set(const Vec<N-1, Tv>& v, const Ts& s){ back()=s; return (*this = v); }

	template <class Ts, class Tv>
	Vec& set(const Ts& s, const Vec<N-1, Tv>& v){ front()=s; sub<N-1,1>()=v; return *this; }

	/// Set elements from (strided) raw C-pointer
	template <class T2>
	Vec& set(const T2 * const v, int stride=1){ IT(N) at(i) = T(v[i*stride]); return *this; }

	/// Set first 2 elements
	Vec& set(const T& v1, const T& v2){
		static_assert(N==2, "Attempt to set vector with wrong number of elements");
		at<0>()=v1; at<1>()=v2;
		return *this;
	}

	/// Set first 3 elements
	Vec& set(const T& v1, const T& v2, const T& v3){
		static_assert(N==3, "Attempt to set vector with wrong number of elements");
		at<0>()=v1; at<1>()=v2; at<2>()=v3;
		return *this;
	}

	/// Set first 4 elements
	Vec& set(const T& v1, const T& v2, const T& v3, const T& v4){
		static_assert(N==4, "Attempt to set vector with wrong number of elements");
		at<0>()=v1; at<1>()=v2; at<2>()=v3; at<3>()=v4;
		return *this;
	}

	/// Set first 5 elements
	Vec& set(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5){
		static_assert(N==5, "Attempt to set vector with wrong number of elements");
		at<0>()=v1; at<1>()=v2; at<2>()=v3; at<3>()=v4; at<4>()=v5;
		return *this;
	}

	/// Set first 6 elements
	Vec& set(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6){
		static_assert(N==6, "Attempt to set vector with wrong number of elements");
		at<0>()=v1; at<1>()=v2; at<2>()=v3; at<3>()=v4; at<4>()=v5; at<5>()=v6;
		return *this;
	}

	/// Set elements from initializer list {a,b,...}

	/// If the initializer list has one element, then it is assigned to all
	/// vector components.
	Vec& set(std::initializer_list<T> v){
		if(v.size() == 1){
			*this = v.begin()[0];
		} else {
			const int M = N<v.size() ? N : int(v.size());
			for(int i=0; i<M; ++i) at(i) = v.begin()[i];
			for(int i=M; i<N; ++i) at(i) = T();
		}
		return *this;
	}

	/// Fill subrange of elements with value
	Vec& fill(const T& v, int count=N, int begin=0){
		for(int i=begin; i<begin+count; ++i) at(i) = v;
		return *this;
	}

	/// Fill last 'count' elements with value
	Vec& fillLast(const T& v, int count){ return fill(v, count, N-count); }

	/// Set to axis-aligned vector
	Vec& setAA(int axis, T val = T(1)){
		*this = T(0);
		at(axis) = val;
		return *this;
	}
	template <int Axis>
	Vec& setAA(T val = T(1)){
		static_assert(0<=Axis && Axis<N, "Axis out of range");
		return setAA(Axis, val);
	}


	/// Return true if objects are element-wise equal, false otherwise
	bool operator ==(const Vec& v) const { IT(N){ if(at(i) != v[i]) return false; } return true; }

	/// Return true if all elements are equal to value, false otherwise
	bool operator ==(const   T& v) const { IT(N){ if(at(i) != v   ) return false; } return true; }

	/// Return true if objects are not element-wise equal, false otherwise
	bool operator !=(const Vec& v) const { return !(*this == v); }

	/// Return true if all elements are not equal to value, false otherwise
	bool operator !=(const   T& v) const { return !(*this == v); }

	/// Get a vector comprised of indexed elements
	Vec<2,T> get(int i0, int i1) const {
		return Vec<2,T>(at(i0), at(i1)); }

	/// Get a vector comprised of indexed elements
	Vec<3,T> get(int i0, int i1, int i2) const {
		return Vec<3,T>(at(i0), at(i1), at(i2)); }

	/// Get a vector comprised of indexed elements
	Vec<4,T> get(int i0, int i1, int i2, int i3) const {
		return Vec<4,T>(at(i0), at(i1), at(i2), at(i3)); }

	/// Get a vector comprised of indexed elements (compile-time checked)
	template <int... Indices>
	Vec<sizeof...(Indices),T> get(){
		return toVec(at<Indices>()...);
	}

	/// Get a vector comprised of elements indexed from argument (like APL Index)
	template <int M>
	Vec<M,T> get(const Vec<M,int>& indices) const {
		return indices.template map<T>([this](int i){ return at(i); });
	}

	/// Get new vector with element set to value
	template <int i>
	Vec with(T v = T()) const { return dup().template set<i>(v); }

	/// Get temp copy
	Vec dup() const { return *this; }

	/// Get a subvector
	template <int M, int Begin=0>
	const Vec<M,T>& sub() const {
		return const_cast<Vec*>(this)->sub<M,Begin>();
	}
	template <int M, int Begin=0>
	Vec<M,T>& sub(){
		static_assert((Begin+M)<=N, "Invalid subvector range");
		return Vec<M,T>::pun(elems()+Begin);
	}

	const Vec<2,T>& xy() const { return sub<2>(); }
	Vec<2,T>& xy(){ return sub<2>(); }
	const Vec<2,T>& yz() const { return sub<2,1>(); }
	Vec<2,T>& yz(){ return sub<2,1>(); }
	const Vec<2,T>& zw() const { return sub<2,2>(); }
	Vec<2,T>& zw(){ return sub<2,2>(); }
	const Vec<3,T>& xyz() const { return sub<3>(); }
	Vec<3,T>& xyz(){ return sub<3>(); }
	const Vec<3,T>& yzw() const { return sub<3,1>(); }
	Vec<3,T>& yzw(){ return sub<3,1>(); }

	/// Return new vector with one element erased
	Vec<N-1,T> erase(int index) const {
		Vec<N-1,T> r(VEC_NO_INIT);
		for(int i=0;     i<index;    ++i) r[i] = at(i);
		for(int i=index; i<r.size(); ++i) r[i] = at(i+1);
		return r;
	}

	/// Remove leading or trailing elements

	/// \tparam M	Number of elements to drop from head (if positive) or tail
	///				(if negative).
	/// This function is based on the APL function of the same name.
	template <int M, int L = N-const_min(const_abs(M), N)>
	Vec<L,T> drop() const {
		return sub<L, M<0?0:N-L>();
	}

	/// Get new vector composed of subsequence of this vector

	/// \tparam M	The size of the new vector is |M|. For M>0 or M<0, as many
	///				elements as possible are copied from the front or back,
	///				respectively. When |M|>N, "overtaking" occurs and extra
	///				elements are filled with the argument value.
	/// Examples:\n
	/// \code
	///		Vec(1,2,3,4,5).take< 3>() == Vec(1,2,3)
	///		Vec(1,2,3,4,5).take<-3>() == Vec(3,4,5)
	///		Vec(1,2,3).take< 5>(-1) == Vec(1,2,3,-1,-1)
	/// 	Vec(1,2,3).take<-5>(-1) == Vec(-1,-1,1,2,3)
	/// \endcode
	/// This function is based on the APL function of the same name.
	template <int M, int L = const_abs(M)>
	Vec<L,T> take(const T& fill = T()) const {

		constexpr auto Nc = const_min(L, N);
		constexpr auto Bc = M>=0 ? 0 : L-Nc;
		constexpr auto Ec = Bc + Nc;
		constexpr auto Oc = M<0 && L<N ?  L-N : Bc;

		constexpr auto Nf = L<=N ? 0 : L-N;
		constexpr auto Bf = M>=0 ? Bc+Nc : 0;
		constexpr auto Ef = Bf + Nf;

		Vec<L,T> r(VEC_NO_INIT);
		for(int i=Bc; i<Ec; ++i) r[i] = at(i-Oc);
		for(int i=Bf; i<Ef; ++i) r[i] = fill;
		return r;
	}

	/// Returns self with other vector inserted

	/// \tparam I	Position in vector before which new elements are inserted
	///
	template <int I, int M, class U>
	Vec<N+M,T> insert(const Vec<M,U>& v) const {
		static constexpr auto J = I>=0 ? I : N+I+1;
		static_assert(J>=0 && J<=N, "Invalid insertion position");
		Vec<N+M,T> r(VEC_NO_INIT);
		for(int i=0  ; i<J  ; ++i) r[i] = at(i);
		for(int i=J  ; i<J+M; ++i) r[i] = v[i-J];
		for(int i=J+M; i<N+M; ++i) r[i] = at(i-M);
		return r;
	}
	template <int I, class S>
	Vec<N+1,T> insert(const S& s) const {
		return insert<I>(Vec<1,T>(s));
	}

	/// Returns self concatenated with other
	template <int M, class U>
	Vec<N+M,T> concat(const Vec<M,U>& v) const {
		Vec<N+M,T> r(VEC_NO_INIT);
		for(int i=0; i<N; ++i) r[i  ] = at(i);
		for(int i=0; i<M; ++i) r[i+N] = T(v[i]);
		return r;
	}

	template <class S>
	Vec<N+1,T> concat(const S& s) const {
		return Vec<N+1,T>(*this,s);
	}

	/// Returns self interleaved with other
	template <int SelfOffset = 0>
	Vec<N+N,T> interleave(const Vec<N,T>& v) const {
		static_assert(SelfOffset==0 || SelfOffset==1, "Offset must be 0 or 1");
		static constexpr auto OtherOffset = 1-SelfOffset;
		Vec<N+N,T> r(VEC_NO_INIT);
		for(int i=0; i<N; ++i){
			r[2*i+ SelfOffset] = at(i);
			r[2*i+OtherOffset] = v[i];
		}
		return r;
	}

	/// Get vector as vector of vectors
	template <int M>
	const Vec<N/M, Vec<M,T>>& nest() const {
		return const_cast<Vec*>(this)->nest<M>();  
	}

	template <int M>
	Vec<N/M, Vec<M,T>>& nest(){
		static constexpr auto N_M = N/M;
		static_assert(N_M * M == N, "Invalid divisor: N/M must be an integer");
		return as<Vec<N_M, Vec<M,T>>>();
	}

	/// Reverse element order
	Vec& reverse(){
		static constexpr auto N_2 = N/2;
		for(int i=0; i<N_2; ++i) swap(i, N-1-i);
		return *this;
	}

	/// Apply circular shift to elements

	/// Element i is moved to element (i+D)%N.
	///
	template <int D>
	Vec& cshift(){
		constexpr auto Dp = const_mods(D,N);
		return *this = drop<N-Dp>().concat(drop<-Dp>());
	}

	/// Swap elements within vector
	Vec& swap(int i, int j){ std::swap(at(i), at(j)); return *this; }

	template <int i=0, int j=1>
	Vec& swap(){ std::swap(at<i>(), at<j>()); return *this; }

	/// Swap elements with another vector
	Vec& swap(Vec& v){ IT(N) std::swap(at(i), v[i]); return *this; }


	//--------------------------------------------------------------------------
	// Basic Arithmetic Operations

	#define DEF_VEC_OP(op)\
	template <class U>\
	Vec& operator op##=(const Vec<N,U>& v){ IT(N) at(i) op##= v[i]; return *this; }\
	Vec& operator op##=(const        T& v){ IT(N) at(i) op##= v;    return *this; }\
	template <class U>\
	Vec operator op (const Vec<N,U>& v) const { return dup() op##= v; }\
	Vec operator op (const        T& v) const { return dup() op##= v; }
	
	DEF_VEC_OP(+)
	DEF_VEC_OP(-)
	DEF_VEC_OP(*)
	DEF_VEC_OP(/)

	#undef DEF_VEC_OP

	#define DEF_VEC_ONESIES(op)\
	Vec& operator op##op (){ return *this op##= T(1); }\
	Vec operator op##op (int){ auto t = *this; op##op*this; return t; }

	DEF_VEC_ONESIES(+)
	DEF_VEC_ONESIES(-)

	#undef DEF_VEC_ONESIES

	Vec operator - () const { return dup().negate(); }
	Vec operator + () const { return *this; }
	bool operator > (const Vec& v) const { return magSqr() > v.magSqr(); }
	bool operator < (const Vec& v) const { return magSqr() < v.magSqr(); }

	/// Square vector
	Vec& square(){ return *this *= *this; }
	/// Get squared vector
	Vec squared() const { return dup().square(); }

	/// Apply a function in-place on each element
	template <class Func, class... Args>
	Vec& apply(Func func, Args... args){
		for(auto& v : *this) func(v, args...);
		return *this;
	}

	template <class V, class Func, class... Args>
	Vec<N,V> map(Func func, Args... args) const {
		Vec<N,V> r(VEC_NO_INIT);
		for(int i=0; i<size(); ++i) r[i] = func(at(i), args...);
		return r;
	}

	/// Map elements through function into new vector

	/// @param[in] func		Function taking old value and returning new value
	/// @param[in] args		Extra function arguments
	template <class Func, class... Args>
	Vec map(Func func, Args... args) const {
		return map<T>(func, args...);
	}

	/// Map element indices through function into new vector
	template <class Func, class... Args>
	static Vec<N,T> mapIndex(Func func, Args... args){
		Vec<N,T> r(VEC_NO_INIT);
		for(int i=0; i<size(); ++i) r[i] = func(i, args...);
		return r;
	}

	/// Reduce elements into scalar

	/// @param[in] prev		Initial previous value
	/// @param[in] func		Function taking previous and current values as first 
	///						two arguments and returning new value
	/// @param[in] args		Extra function arguments
	template <class Func, class... Args>
	T reduce(const T& prev, Func func, Args... args) const {
		T r = prev;
		for(auto& v : *this) r = func(r, v, args...);
		return r;
	}


	//--------------------------------------------------------------------------
	// Linear Operations

	/// Returns a nearby vector along some dimension

	/// @tparam 	Dimension	The dimension along which to get a nearby vector
	/// @param[in]	shift		The amount to shift along specified dimension
	template<int Dimension>
	Vec by(T shift) const {
		static_assert(Dimension<N, "Dimension out of bounds");
		Vec res(*this);
		res[Dimension] += shift;
		return res;
	}

	/// Returns a nearby vector along x
	Vec byx(T shift) const { return by<0>(shift); }

	/// Returns a nearby vector along y
	Vec byy(T shift) const { return by<1>(shift); }

	/// Returns a nearby vector along z
	Vec byz(T shift) const { return by<2>(shift); }

	/// Returns dot (inner) product between vectors
	template <class U>
	T dot(const Vec<N,U>& v) const {
		T r = front() * v.front();
		for(int i=1; i<N; ++i){ r += at(i) * v[i]; }
		return r;
	}

	/// Returns magnitude
	T mag() const { return std::sqrt(magSqr()); }

	/// Returns magnitude squared
	T magSqr() const { return dot(*this); }

	/// Get polar angle on plane, in [-pi,pi] radians
	template <int Dim1=0, int Dim2=1>
	T angle() const {
		static_assert_dims<Dim1,Dim2>();
		return std::atan2(at<Dim2>(), at<Dim1>());
	}

	/// Get polar angle on plane, in [0,1]
	template <int Dim1=0, int Dim2=1>
	T angleUnit() const {
		auto a = angle<Dim1,Dim2>() * 0.1591549430919;
		return a >= 0. ? a : a + 1.;
	}

	/// Returns vector filled with absolute values of elements
	Vec absVec() const {
		using namespace std;
		Vec res(*this);
		for(auto& v : res) v=abs(v);
		return res;
	}

	/// Returns component-wise signum function
	Vec sgn(T mag=T(1)) const {
		Vec s;
		for(int i=0; i<N; ++i){
			const auto& v = at(i);
			s[i] = v<T(0) ? -mag : v>T(0) ? mag : T(0);
		}
		return s;
	}

	/// Get vector filled with differences between successive elements
	Vec delta(T endVal = T(1)) const {
		Vec r(VEC_NO_INIT);
		for(int i=0; i<N-1; ++i){
			r[i] = at(i+1) - at(i);
		}
		r.back() = endVal - back();
		return r;
	}

	/// Returns p-norm of elements

	/// The p-norm is pth root of the sum of the absolute value of the elements
	/// raised to the pth power, (sum |x_n|^p) ^ (1/p). See norm1 and norm2 for
	/// more efficient specializations.
	T norm(const T& p) const {
		return std::pow(pow(absVec(), p).sum(), T(1)/p);
	}

	/// Return 1-norm of elements (sum of absolute values)
	T norm1() const { return sumAbs(); }

	/// Return 2-norm of elements
	T norm2() const { return mag(); }

	/// Returns product of elements
	T product() const {
		T r = front();
		for(int i=1; i<N; ++i){ r *= at(i); }
		return r;
	}

	/// Returns sum of elements
	T sum() const {
		T r = front();
		for(int i=1; i<N; ++i){ r += at(i); }
		return r;
	}

	/// Returns sum of absolute value of elements (1-norm)
	T sumAbs() const { return absVec().sum(); }

	/// Returns mean (average) of elements
	T mean() const {
		static constexpr float invSize = 1./size();
		return sum()*invSize;
	}

	/// Linearly interpolate towards some target
	Vec& lerp(const Vec& target, T amt){
		return *this += (target-*this)*amt;
	}

	/// Set magnitude (preserving direction)
	Vec& mag(T v);

	/// Set 1-norm of vector (sum of absolute values)

	/// This is useful for when you need to ensure the sum of absolute values 
	/// is equal to some value without changing the direction of the vector.
	Vec& norm1(T v){
		auto n1 = sumAbs();
		if(n1 > T(0)) *this *= v/n1;
		return *this;
	}

	/// Negates all elements
	Vec& negate(){
		for(auto& v:*this){ v = -v; } return *this;
	}

	/// Normalize magnitude (preserving direction)

	/// @param[in] magVal	magnitude (1 is a standard normalization)
	///
	Vec& normalize(T magVal=T(1)){
		return mag(magVal);
	}

	/// Return closest vector lying on a sphere

	/// @param[in] magVal	magnitude (1 is a standard normalization)
	///
	Vec normalized(T magVal=T(1)) const {
		return dup().normalize(magVal); }

	/// Get projection of vector onto a unit vector
	Vec projection(const Vec& u) const {
		return dot(u) * u;
	}

	/// Get rejection of vector from a unit vector

	/// This also gives the projection onto a plane defined by normal 'u'.
	///
	Vec rejection(const Vec& u) const {
		return *this - projection(u);
	}

	/// Returns whether this is inside sphere

	/// @param[in] p		center of sphere
	/// @param[in] radius	radius of sphere
	bool insideSphere(const Vec& p, T radius) const {
		return (*this-p).magSqr() < radius*radius;
	}

	/// Returns whether this is outside sphere

	/// @param[in] p		center of sphere
	/// @param[in] radius	radius of sphere
	bool outsideSphere(const Vec& p, T radius) const {
		return !insideSphere(p, radius);
	}

	/// Returns whether point is inside sphere centered at origin
	template <int Rnum=1, int Rden=1>
	bool insideSphere() const {
		static_assert(Rden != 0, "Divide by zero");
		static constexpr T R = T(Rnum)/Rden;
		static constexpr T Rsqr = R*R;
		return magSqr() < Rsqr;
	}

	/// Returns whether point is outside sphere centered at origin
	template <int Rnum=1, int Rden=1>
	bool outsideSphere() const { return !insideSphere<Rnum,Rden>(); }

	/// Get projection onto sphere

	/// This function is useful for applying distance-based constraints.
	/// @param[in] p		center of sphere
	/// @param[in] radius	radius of sphere
	Vec projSphere(const Vec& p, T radius) const {
		return (*this-p).normalize(radius) + p;
	}

	/// Reflect vector around a unit vector
	Vec& reflect(const Vec& u){
		return *this -= ((T(2) * dot(u)) * u);
	}

	/// Set from angle and magnitude
	template <int Dim1=0, int Dim2=1>
	Vec& fromPolar(double ang, double mag = 1.){
		static_assert_dims<Dim1,Dim2>();
		at<Dim1>() = mag * std::cos(ang);
		at<Dim2>() = mag * std::sin(ang);
		return *this;
	}

	Vec& rotate(double cosAng, double sinAng, int dim1, int dim2){
		T t = at(dim1);
		T u = at(dim2);
		at(dim1) = t*cosAng - u*sinAng;
		at(dim2) = t*sinAng + u*cosAng;
		return *this;
	}

	template <unsigned Dim1=0, unsigned Dim2=1>
	Vec& rotate(double cosAng, double sinAng){
		static_assert_dims<Dim1,Dim2>();
		return rotate(cosAng,sinAng, Dim1,Dim2);
	}

	/// Rotate vector on a global plane

	/// @param[in] angle	angle of right-handed rotation, in radians
	/// @param[in] dim1		dimension to rotate from
	/// @param[in] dim2		dimension to rotate towards
	Vec& rotate(double angle, int dim1, int dim2){
		return rotate(std::cos(angle),std::sin(angle), dim1,dim2);
	}

	template <unsigned Dim1=0, unsigned Dim2=1>
	Vec& rotate(double angle){
		static_assert_dims<Dim1,Dim2>();
		return rotate(angle, Dim1,Dim2);
	}

	/// Rotate vector 45 degrees on a global plane

	/// @param[in] dim1		dimension to rotate from
	/// @param[in] dim2		dimension to rotate towards
	/// To rotate -45 degrees, swap the two dimensions.
	Vec& rotate45(int dim1, int dim2){
		/*	    | c -c |
			R = | c  c | where c = sqrt(2)/2

			R p = { ax - ay, ax + ay } */
		float c = 0.7071067811865475244; // sqrt(2)/2
		auto x = at(dim1) * c;
		auto y = at(dim2) * c;
		at(dim1) = x-y;
		at(dim2) = x+y;
		return *this;
	}

	template <unsigned Dim1=0, unsigned Dim2=1>
	Vec& rotate45(){
		static_assert_dims<Dim1,Dim2>();
		return rotate45(Dim1,Dim2);
	}

	/// Rotate vector 90 degrees on a global plane

	/// @param[in] dim1		dimension to rotate from
	/// @param[in] dim2		dimension to rotate towards
	/// To rotate -90 degrees, swap the two dimensions.
	Vec& rotate90(int dim1, int dim2){
		at(dim2) = -at(dim2);
		return swap(dim1, dim2);
	}

	template <unsigned Dim1=0, unsigned Dim2=1>
	Vec& rotate90(){
		static_assert_dims<Dim1,Dim2>();
		return rotate90(Dim1,Dim2);
	}

	/// Rotate vector 180 degrees on a global plane

	/// @param[in] dim1		dimension to rotate from
	/// @param[in] dim2		dimension to rotate towards
	Vec& rotate180(int dim1, int dim2){
		at(dim1) = -at(dim1);
		at(dim2) = -at(dim2);
		return *this;
	}

	template <unsigned Dim1=0, unsigned Dim2=1>
	Vec& rotate180(){
		static_assert_dims<Dim1,Dim2>();
		return rotate180(Dim1,Dim2);
	}


	//--------------------------------------------------------------------------
	// Analysis

	/// Returns index of first occurrence of value or -1 if no match
	int find(const T& v) const {
		for(int i=0; i<N; ++i){ if(v == at(i)) return i; }
		return -1;
	}

	/// Get index of minimum value
	int indexOfMin() const {
		int j = 0;
		for(int i=1; i<N; ++i){
			if(at(i) < at(j)) j=i;
		}
		return j;
	}

	/// Get index of maximum value
	int indexOfMax() const {
		int j = 0;
		for(int i=1; i<N; ++i){
			if(at(i) > at(j)) j=i;
		}
		return j;
	}

	/// Get indices of minimum and maximum values
	Vec<2,int> indicesOfExtrema() const {
		int j = 0, J = 0;
		for(int i=1; i<N; ++i){
			if(at(i) < at(j)) j=i;
			if(at(i) > at(J)) J=i;
		}
		return {j, J};
	}

	/// Get minimum value
	const T& min() const { return const_cast<Vec*>(this)->min(); }
	T& min(){ return at(indexOfMin()); }

	/// Get maximum value
	const T& max() const { return const_cast<Vec*>(this)->max(); }
	T& max(){ return at(indexOfMax()); }

	/// Get minimum and maximum values
	Vec<2,T> extrema() const { return get(indicesOfExtrema()); }


	struct PSeq{
		T * data;
		unsigned index = N-1;

		PSeq(T * ptr): data(ptr){}

		T& operator()(){
			index = (index+1) % N;
			return data[index];
		}
	};

	PSeq pseq(){ return PSeq(elems()); }


	/// debug printing
	void print(FILE * out=stdout, const char * append="") const;
	void println(FILE * out=stdout) const;

private:
	// set last N-M elements to default value
	template <int M>
	void initTail(){
		for(int i=M; i<size(); ++i) at(i) = T();
	}

	template <unsigned Dim1, unsigned Dim2>
	static constexpr int static_assert_dims(){
		static_assert(Dim1<N && Dim2<N && Dim1!=Dim2, "Invalid dimension(s)");
		return 0;
	}
};




// -----------------------------------------------------------------------------
// The following are functions that either cannot be defined as class methods
// (due to syntax rules or specialization) or simply are not object oriented.

// Non-member binary arithmetic operations
template <int N, class T>
inline Vec<N,T> operator + (const T& s, const Vec<N,T>& v){ return  v+s; }

template <int N, class T>
inline Vec<N,T> operator - (const T& s, const Vec<N,T>& v){ return -v+s; }

template <int N, class T>
inline Vec<N,T> operator * (const T& s, const Vec<N,T>& v){ return  v*s; }

template <int N, class T>
inline Vec<N,T> operator / (const T& s, const Vec<N,T>& v){
	auto r=v; for(auto& e:r) e = s/e; return r;
}


// Specialized vector functions

/// Returns concatenation of two vectors
template <int N1, class T1, int N2, class T2>
inline Vec<N1+N2, T1> concat(const Vec<N1,T1>& a, const Vec<N2,T2>& b){
	return a.concat(b);
}

template <int N, class T, class V>
inline Vec<N+1, T> concat(const Vec<N,T>& v, const V& s){
	return v.concat(s);
}

template <int N, class T, class V>
inline Vec<N+1, T> concat(const V& s, const Vec<N,T>& v){
	return Vec<N+1,T>(s,v);
}

template <class V>
Vec<1,V> toVec(const V& v){ return {v}; }

/// Returns new Vec filled with values
template <class V, class... Vs>
Vec<1+sizeof...(Vs),V> toVec(const V& v, Vs... vs){
	return concat(v, toVec(vs...));
}

/// Get a subvector
template <int M, int Begin, int N, class T>
inline Vec<M,T> sub(const Vec<N,T>& v){
	return v.template sub<M,Begin>();
}
template <int M, int Begin, int N, class T>
inline Vec<M,T>& sub(Vec<N,T>& v){
	return v.template sub<M,Begin>();
}
template <int M, int N, class T>
inline Vec<M,T> sub(const Vec<N,T>& v){
	return sub<M,0>(v);
}
template <int M, int N, class T>
inline Vec<M,T>& sub(Vec<N,T>& v){
	return sub<M,0>(v);
}

/// Returns vector containing element-wise minimum between two vectors
template <int N, class T>
inline Vec<N,T> min(const Vec<N,T>& a, const Vec<N,T>& b){
	return a.mapIndex([&](int i){ return a[i] < b[i] ? a[i] : b[i]; });
}
template <int N, class T>
inline Vec<N,T> min(const Vec<N,T>& a, const T& b){
	return min(a, Vec<N,T>(b));
}
template <int N, class T>
inline Vec<N,T> min(const T& a, const Vec<N,T>& b){
	return min(b, a);
}

/// Returns vector containing element-wise maximum between two vectors
template <int N, class T>
inline Vec<N,T> max(const Vec<N,T>& a, const Vec<N,T>& b){
	return a.mapIndex([&](int i){ return a[i] > b[i] ? a[i] : b[i]; });
}
template <int N, class T>
inline Vec<N,T> max(const Vec<N,T>& a, const T& b){
	return max(a, Vec<N,T>(b));
}
template <int N, class T>
inline Vec<N,T> max(const T& a, const Vec<N,T>& b){
	return max(b, a);
}

/// Get vector with absolute value of each element
template <int N, class T>
inline Vec<N,T> abs(const Vec<N,T>& v){
	auto r=v; for(auto& e:r) e = std::abs(e);
	return r;
}

/// Get vector with each element raised to a power
template <int N, class T>
inline Vec<N,T> pow(const Vec<N,T>& v, const T& power){
	auto r=v; for(auto& e:r) e = std::pow(e, power);
	return r;
}

/// Get vector with each element raised to a power
template <int N, class T>
inline Vec<N,T> pow(const T& base, const Vec<N,T>& powers){
	auto r=powers; for(auto& e:r) e = std::pow(base, e);
	return r;
}

template <int N, class T>
inline Vec<N,T> ceil(const Vec<N,T>& v){ return v.map([](T x){ return std::ceil(x); }); }

template <int N, class T>
inline Vec<N,T> ceil(const Vec<N,T>& v, const Vec<N,T>& step){ return ceil(v/step)*step; }

template <int N, class T>
inline Vec<N,T> ceil(const Vec<N,T>& v, const T& step){ return ceil(v/step)*step; }

template <int N, class T>
inline Vec<N,T> floor(const Vec<N,T>& v){ return v.map([](T x){ return std::floor(x); }); }

template <int N, class T>
inline Vec<N,T> floor(const Vec<N,T>& v, const Vec<N,T>& step){ return floor(v/step)*step; }

template <int N, class T>
inline Vec<N,T> floor(const Vec<N,T>& v, const T& step){ return floor(v/step)*step; }

/// Get real-valued vector rounded to nearest integer vector
template <int N, class T>
inline Vec<N,int> roundi(const Vec<N,T>& v){ return v + v.sgn(T(0.5)); }

/// Returns sum of elements
template <int N, class T>
inline T sum(const Vec<N,T>& v){ return v.sum(); }

/// Sets r to cross product, a x b
template <class T>
inline void cross(Vec<3,T>& r, const Vec<3,T>& a, const Vec<3,T>& b){
//	r[0] = a[1]*b[2] - a[2]*b[1];
//	r[1] = a[2]*b[0] - a[0]*b[2];
//	r[2] = a[0]*b[1] - a[1]*b[0];
	r = a.cross(b);
}

/// Returns cross product, a x b
template <class T>
inline Vec<3,T> cross(const Vec<3,T>& a, const Vec<3,T>& b){
	Vec<3,T> r;	cross(r,a,b); return r;
}

/// Returns dot product
template <int N, class T>
inline T dot(const Vec<N,T>& a, const Vec<N,T>& b){
	return a.dot(b);
}

/// Rotate a vector around a normal vector

/// @param[in,out]	vec		Vector to rotate
/// @param[in]		normal	Normal perpendicular to the plane of rotation
/// @param[in]		cosAng	Cosine of the rotation angle
/// @param[in]		sinAng	Sine of the rotation angle
template <class T>
void rotate(Vec<3,T>& vec, const Vec<3,T>& normal, double cosAng, double sinAng){
	T c = cosAng;
	T s = sinAng;

	// Rodrigues' rotation formula:
	vec = vec*c + cross(normal, vec)*s + normal*(normal.dot(vec)*(T(1)-c));
}

/// Rotate a vector around a normal vector

/// @param[in,out]	vec			Vector to rotate
/// @param[in]		normal		Normal perpendicular to the plane of rotation
/// @param[in]		angle		Rotation angle, in radians
template <class T>
void rotate(Vec<3,T>& vec, const Vec<3,T>& normal, double angle){
	rotate(vec, normal, cos(angle), sin(angle));
}

template <class T>
Vec<3,T> rotated(const Vec<3,T>& vec, const Vec<3,T>& normal, double cosAng, double sinAng){
	auto r = vec; rotate(r, normal, cosAng, sinAng); return r;
}
template <class T>
Vec<3,T> rotated(const Vec<3,T>& vec, const Vec<3,T>& normal, double angle){
	return rotated(vec, normal, cos(angle), sin(angle));
}


/// Rotate a vector 90 degrees around a normal vector

/// @param[in,out]	vec		Vector to rotate
/// @param[in]		normal	Normal perpendicular to the plane of rotation
template <class T>
void rotate90(Vec<3,T>& vec, const Vec<3,T>& normal){
	// Simplified Rodrigues' rotation formula with c=0,s=1:
	//vec = vec*c + cross(normal, vec)*s + normal*(normal.dot(vec)*(T(1)-c));
	vec = cross(normal, vec) + vec.projection(normal);
}

template <class T>
Vec<3,T> rotated90(const Vec<3,T>& vec, const Vec<3,T>& normal){
	auto r = vec; rotate90(r, normal); return r;
}


/// Returns angle, in interval [0, pi], between two vectors
template <int N, class T>
inline T angle(const Vec<N,T>& a, const Vec<N,T>& b){
	T cosAng = a.dot(b) / sqrt(a.magSqr()*b.magSqr());
	if(cosAng >= T( 1)){ return T(0); }		else
	if(cosAng <= T(-1)){ return T(3.141592653589793); }
	return std::acos(cosAng);
}

/*! Compute centroid of points
	@param[ in] p1	Point 1
	@param[ in] p2	Point 2
	@param[ in] p3	Point 3
	\returns centroid of points
*/
template <int N, class T>
inline Vec<N,T> centroid(const Vec<N,T>& p1, const Vec<N,T>& p2, const Vec<N,T>& p3){
	return (p1+p2+p3)/T(3);
}

/// Get closest point on a line to a point p
template <int N, class T>
Vec<N,T> closestPointOnLine(const Vec<N,T>& linePnt, const Vec<N,T>& lineDir, const Vec<N,T>& p){
	return linePnt + (p-linePnt).projection(lineDir);
}

template <int N, class T, class Tf>
Vec<N,T> closestPointOnLineSegment(Tf& frac, const Vec<N,T>& a, const Vec<N,T>& b, const Vec<N,T>& p){
	auto ab = b - a;
	auto dot = (p - a).dot(ab);	// projection of ap onto ab
	auto magAB = ab.magSqr();

	// normalized distance along ab from a to the closest point  
	frac = magAB > T(0) ? dot / magAB : T(0);

	// check if p projection is beyond endpoints of ab   
	if(frac <= T(0)) return a;
	if(frac >= T(1)) return b;

	return a + ab * frac;
}

/// Get closest point on line segment ab to point p
template <int N, class T>
Vec<N,T> closestPointOnLineSegment(const Vec<N,T>& a, const Vec<N,T>& b, const Vec<N,T>& p){
	T f; return closestPointOnLineSegment(f, a,b,p);
}

/// Line-plane intersection test

/// @param[out] d	scalar indicating intersection on line d*l + l0
/// @param[in] l	direction of line d*l + l0
/// @param[in] l0	point on line d*l + l0
/// @param[in] n	normal of plane
/// @param[in] p0	point on plane
/// \returns 1 if single intersection, 0 if no intersection or 2 if line on plane
template <int N, class T, class D>
int linePlaneIntersection(
	D& d,
	const Vec<N,T>& l, const Vec<N,T>& l0,
	const Vec<N,T>& n, const Vec<N,T>& p0
){
	auto a = (p0-l0).dot(n);
	auto b = l.dot(n);
	if(b == T(0)){ // line and plane parallel
		if(a == T(0)) return 2; // line on plane
		return 0;
	}
	d = a / b;
	return 1;
}

/// Returns distance between two vectors
template <int N, class T, class U>
inline T dist(const Vec<N,T>& a, const Vec<N,U>& b){
	return (a-b).mag();
}

/// Return magnitude of vector (GLSL compatibility)
template <int N, class T>
inline T length(const Vec<N,T>& v){ return v.mag(); }

/// Return normalized vector (GLSL compatibility)
template <int N, class T>
inline Vec<N,T> normalize(const Vec<N,T>& v){ return v.normalized(); }

template <int N, class T>
inline Vec<N,T> lerp(const Vec<N,T>& input, const Vec<N,T>& target, T amt){
	return Vec<N,T>(input).lerp(target, amt);
}

/// Get normal to triangle defined by three points in CCW order
template <class T>
inline Vec<3,T> normal(const Vec<3,T>& p1, const Vec<3,T>& p2, const Vec<3,T>& p3){
	return cross(p2-p1, p3-p1).normalize();
}


/// Get surface normal given 2D gradient (useful for terrain rendering)
template <class T>
inline Vec<3,T> gradToNormal(const Vec<2,T>& g){
	// result is normalized (-g.x, -g.y, 1)
	return g.template take<3>(T(-1)).normalized(T(-1));
}

/// Swap elements between vectors
template <int N, class T>
inline void swap(Vec<N,T>& a, Vec<N,T>& b){ a.swap(b); }

/// Pun uniform POD into vector
template <class T, class UniformPOD>
auto punToVec(UniformPOD& v) -> Vec<sizeof(UniformPOD)/sizeof(T),T>& {
	static_assert(!std::is_polymorphic<UniformPOD>::value, "Punning polymorphic class disallowed");
	return std::remove_reference<decltype(punToVec<T>(v))>::type::pun((T*)&v);
}

template <class T, class UniformPOD>
auto punToVec(const UniformPOD& v) -> const Vec<sizeof(UniformPOD)/sizeof(T),T>& {
	return punToVec<T>(const_cast<UniformPOD&>(v));
}



// Implementation --------------------------------------------------------------

template <int N, class T>
Vec<N,T>& Vec<N,T>::mag(T v){
	T m = mag();
	if(m > T(1e-20)){
		*this *= v/m;
	}
	else{
		*this = T(0);
		front() = v;
	}
	return *this;
}

template<int N, class T>
void Vec<N,T>::print(FILE * out, const char * append) const {
	fprintf(out, "{");
	if(size()){
		fprintf(out, "%g", (double)(front()));
		for (int i=1; i<N; ++i)
			fprintf(out, ", %g", (double)(at(i)));
	}
	fprintf(out, "}%s", append);
}

template<int N, class T>
void Vec<N,T>::println(FILE * out) const { print(out, "\n"); }

template <int N, class T>
std::ostream & operator << (std::ostream & out, const Vec<N,T> &v) {
	out << "{";
	if(v.size()){
		out << v[0];
		for (int i = 1; i < N; ++i)
			out << ", " << v[i];
	}
	out << "}";
	return out;
}

#undef IT
} // al::
#endif
