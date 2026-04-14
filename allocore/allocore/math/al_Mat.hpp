#ifndef INC_AL_MAT_HPP
#define INC_AL_MAT_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Generic fixed-size NxN matrix

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <cmath>
#include <cstdio>
#include <ostream>
#include "allocore/math/al_Vec.hpp"

namespace al {

template <int N, class T> class Mat;

typedef Mat<2,float>	Mat2f;	///< float 2x2 matrix
typedef Mat<2,double>	Mat2d;	///< double 2x2 matrix
typedef Mat<2,int>		Mat2i;	///< integer 2x2 matrix
typedef Mat<3,float>	Mat3f;	///< float 3x3 matrix
typedef Mat<3,double>	Mat3d;	///< double 3x3 matrix
typedef Mat<3,int>		Mat3i;	///< integer 3x3 matrix
typedef Mat<4,float>	Mat4f;	///< float 4x4 matrix
typedef Mat<4,double>	Mat4d;	///< double 4x4 matrix
typedef Mat<4,int>		Mat4i;	///< integer 4x4 matrix

// Forward iterates from 0 to n-1. Current index is 'i'.
#define IT(n) for(int i=0; i<(n); ++i)


/// Flag type to prevent element initialization
static struct MatNoInit{} MAT_NO_INIT;


/// Combined rotation and uniform scaling

/// This class provides a uniform way of representing a rotation and scaling.
/// It is essentially a complex number, z, where arg(z) is the rotation angle
/// and |z| is the scaling amount. The transform is stored in rectangular form
/// for efficiency reasons. When building rotation matrices, we typically need
/// to compute cos(angle) and sin(angle), so these are essentially cached here.
/// The scaling part is optional, but comes for free, so is included.
template <class T>
class Rotoscale{
public:
	using value_type = T;

	T r, i;

	Rotoscale(){}
	Rotoscale(const Rotoscale& v): r(v.r), i(v.i){}
	Rotoscale(T ang){ angle(ang); }
	Rotoscale(T re, T im): r(re), i(im){}

	/// Set from angle in radians
	Rotoscale& angle(T v){ return set(std::cos(v), std::sin(v)); }

	/// Set from angle in turns in [0,1]
	Rotoscale& turn(T v){ return angle(v*T(6.283185307179586)); }

	/// Set from angle in degrees
	Rotoscale& deg (T v){ return angle(v*T(0.017453292519943)); }

	template <int Mul=1>
	Rotoscale& deg90(){
		static constexpr T x = (~Mul&1) * (1-(Mul&2));
		static constexpr T y = ( Mul&1) * (1-(Mul&2));
		return set(x,y);
	}

	/// Set directly from real/imag parts
	Rotoscale& set(T re, T im){ r = re; i = im; return *this; }

	Rotoscale dup() const { return Rotoscale(*this); }

	/// Scale by amount
	Rotoscale& operator*= (T v){ r*=v; i*=v; return *this; }
	Rotoscale operator* (T v) const { return dup() *= v; }

	Rotoscale& operator*= (Rotoscale v){ return set(r*v.r-i*v.i, i*v.r + r*v.i); }
	Rotoscale operator* (Rotoscale v) const { return dup() *= v; }

};

typedef Rotoscale< float> Rotoscalef;
typedef Rotoscale<double> Rotoscaled;


/// Fixed-size n-by-n square matrix

/// Elements are stored in column-major format.
///
/// \ingroup allocore
template <int N, class T>
class Mat{
public:

	typedef T value_type;

	/// Column-major array of elements
	T mElems[N*N];


	//--------------------------------------------------------------------------
	// Constructors

	/// Default constructor that initializes elements to zero
	Mat(){ set(T()); }

	/// Construct without initializing elements
	Mat(const MatNoInit& v){}

	/// \param[in] arr	one dimensional array in column-major
	template <class U>
	Mat(const U * arr){ set(arr); }

	/// \param[in] src	matrix with same dimension, but possibly different type
	template <class U>
	Mat(const Mat<N,U>& src){ set(src.elems()); }

	template <int M, class U>
	Mat(const Mat<M,U>& src){
		*this = src;
	}

	/// Create diagonal matrix

	/// Sets the diagonal elements to the input value and all other elements
	/// to zero. The identity matrix is created by passing in a value of 1.
	/// \param[in] diag		The value to place on the diagonal
	Mat(const T& diag){
		diagonal(diag);
	}

	/// 2x2 matrix constructor with element initialization
	Mat(
		const T& r1c1, const T& r1c2,
		const T& r2c1, const T& r2c2
	){
		set(r1c1, r1c2,
			r2c1, r2c2
		);
	}

	template <class V>
	Mat(const Vec<2,V>& c1, const Vec<2,V>& c2){
		set(c1, c2);
	}

	/// 3x3 matrix constructor with element initialization
	Mat(
		const T& r1c1, const T& r1c2, const T& r1c3,
		const T& r2c1, const T& r2c2, const T& r2c3,
		const T& r3c1, const T& r3c2, const T& r3c3
	){
		set(r1c1, r1c2, r1c3,
			r2c1, r2c2, r2c3,
			r3c1, r3c2, r3c3
		);
	}

	template <class V>
	Mat(const Vec<3,V>& c1, const Vec<3,V>& c2, const Vec<3,V>& c3){
		set(c1, c2, c3);
	}

	/// 4x4 matrix constructor with element initialization
	Mat(
		const T& r1c1, const T& r1c2, const T& r1c3, const T& r1c4,
		const T& r2c1, const T& r2c2, const T& r2c3, const T& r2c4,
		const T& r3c1, const T& r3c2, const T& r3c3, const T& r3c4,
		const T& r4c1, const T& r4c2, const T& r4c3, const T& r4c4
	){
		set(r1c1, r1c2, r1c3, r1c4,
			r2c1, r2c2, r2c3, r2c4,
			r3c1, r3c2, r3c3, r3c4,
			r4c1, r4c2, r4c3, r4c4
		);
	}

	template <class V>
	Mat(const Vec<4,V>& c1, const Vec<4,V>& c2, const Vec<4,V>& c3, const Vec<4,V>& c4){
		set(c1, c2, c3, c4);
	}


	//--------------------------------------------------------------------------
	// Factory Methods

	/// Get identity matrix
	static Mat identity(){
		return Mat(T(1));
	}

	/// Get a rotation transform matrix

	/// \param[in] r		rotation (or angle in radians)
	/// \param[in] dim1		first ("from") basis vector of rotation plane
	/// \param[in] dim2		second ("to") basis vector of rotation plane
	static Mat rotation(const Rotoscale<T>& r, unsigned dim1, unsigned dim2){
		Mat m(T(1));
		m(dim1,dim1) = r.r;
		m(dim2,dim1) = r.i;
		m(dim1,dim2) =-r.i;
		m(dim2,dim2) = r.r;
		return m;
	}

	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat rotation(const Rotoscale<T>& r){
		static_assert_plane<Dim1,Dim2>();
		return rotation(r, Dim1, Dim2);
	}

	/// Get a rotation transform matrix

	/// \param[in] r		rotation (or angle in radians)
	/// \param[in] axis		rotation axis; should be a unit vector
	static Mat<4,T> rotation(const Rotoscale<T>& r, const Vec<3,T>& axis){
		T c = r.r;
		T s = r.i;
		T t = T(1)-c;
		T x = axis[0], y = axis[1], z = axis[2];
		T tx = t*x, ty = t*y, tz = t*z;
		T sx = s*x, sy = s*y, sz = s*z;

		return {
			tx*x + c , tx*y - sz, tx*z + sy, T(0),
			tx*y + sz, ty*y + c , ty*z - sx, T(0),
			tx*z - sy, ty*z + sx, tz*z + c , T(0),
			T(0),T(0),T(0),T(1)
		};
	}

	/// Get a 90-degree rotation transform matrix
	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat rotation90(){
		return rotation<Dim1,Dim2>(Rotoscale<T>().deg90());
	}

	/// Get a 180-degree rotation transform matrix
	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat rotation180(){
		return rotation<Dim1,Dim2>(Rotoscale<T>().template deg90<2>());
	}

	/// Get a scaling transform matrix
	template <int M, class V>
	static Mat scaling(const Vec<M,V>& v){
		static_assert(M < N, "Invalid vector size");
		Mat m(T(1));
		for(int r=0; r<M; ++r) m(r,r) = v[r];
		return m;
	}

	/// Get a scaling transform matrix
	template <class V>
	static Mat scaling(V v){
		return scaling(Vec<N-1,V>(v));
	}

	/// Get a scaling transform matrix
	template <typename... Vals>
	static Mat scaling(Vals... vals){
		return scaling(Vec<sizeof...(Vals),T>(vals...));
	}

	/// Get a translation transform matrix
	template <int M, class V>
	static Mat translation(const Vec<M,V>& v){
		static_assert(M < N, "Invalid vector size");
		Mat m(T(1));
		for(int r=0; r<M; ++r) m(r,N-1) = v[r];
		return m;
	}

	/// Get a translation transform matrix
	template <typename... Vals>
	static Mat translation(Vals... vals){
		return translation(Vec<sizeof...(Vals),T>(vals...));
	}

	/// Get scaling-translation (ST) transform matrix

	/// This returns the transform matrix T*S where the respective matrices are
	/// a translation and scaling. The lumped transform is created with only
	/// 2(N-1) assignments.
	static Mat ST(const Vec<N-1,T>& s, const Vec<N-1,T>& t){
		Mat m = Mat::scaling(s);
		m.col<N-1>().template sub<N-1>() = t;
		return m;
	}

	/// Get translation-scaling (TS) transform matrix

	/// This returns the transform matrix S*T where the respective matrices are
	/// a scaling and translation. The lumped transform is created with only
	/// N-1 multiplies.
	static Mat TS(const Vec<N-1,T>& t, const Vec<N-1,T>& s){
		Mat m = Mat::scaling(s);
		m.col<N-1>().template sub<N-1>() = t*s;
		return m;
	}

	/// Get translation-scaling-translation (TST) transform matrix

	/// This returns the transform matrix T2*S*T1 where the respective matrices
	/// are a translation, scaling and translation. The lumped transform is
	/// created with only N-1 madds.
	static Mat TST(const Vec<N-1,T>& t1, const Vec<N-1,T>& s, const Vec<N-1,T>& t2){
		return Mat::TS(t1,s).translateGlobal(t2);
	}

	/// Get scaling-translation-scaling (STS) transform matrix

	/// This returns the transform matrix S2*T*S1 where the respective matrices
	/// are a scaling, translation and scaling. The lumped transform is created
	/// with only 2(N-1) multiplies.
	static Mat STS(const Vec<N-1,T>& s1, const Vec<N-1,T>& t, const Vec<N-1,T>& s2){
		return Mat::ST(s1,t).scaleGlobal(s2);
	}

	/// Get scaling-translation-scaling-translation (STST) transform matrix

	/// This returns the transform matrix T2*S2*T1*S1 where the respective 
	/// matrices are a translation, scaling, translation and scaling. The lumped
	/// transform is created with only N-1 multiplies and madds.
	static Mat STST(const Vec<N-1,T>& s1, const Vec<N-1,T>& t1, const Vec<N-1,T>& s2, const Vec<N-1,T>& t2){
		return Mat::STS(s1,t1,s2).translateGlobal(t2);
	}

	/// Get scaling-rotation (SR) transform matrix

	/// This returns the transform matrix R*S where the respective matrices are
	/// a rotation and scaling. The lumped transform is created with only four
	/// multiplies versus N^3 if using matrix multiplication.
	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat SR(const Vec<N-1,T>& s, const Rotoscale<T>& r){
		Mat m = Mat::scaling(s);
		m.at<Dim1,Dim1>() = r.r * s.template at<Dim1>(); // SR
		m.at<Dim2,Dim1>() = r.i * s.template at<Dim1>();
		m.at<Dim1,Dim2>() =-r.i * s.template at<Dim2>();
		m.at<Dim2,Dim2>() = r.r * s.template at<Dim2>();
		return m;
	}

	/// Get scaling-rotation-translation (SRT) transform matrix

	/// This returns the transform matrix T*R*S where the respective matrices
	/// are a translation, rotation and scaling. The lumped transform is created
	/// with only four multiplies versus 2(N^3) if using matrix multiplication.
	/// In 2D, this transform forms a complete "model" matrix. In 3D and above,
	/// it may be multiplied on the right by additional rotation matrices.
	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat SRT(const Vec<N-1,T>& s, const Rotoscale<T>& r, const Vec<N-1,T>& t = T(0)){
		Mat m = SR<Dim1,Dim2>(s, r); // SR
		m.col<N-1>().template sub<-1>() = t; // SRT
		return m;
	}

	/// Get translation-scaling-rotation (TSR) transform matrix

	/// This returns the transform matrix R*S*T where the respective matrices
	/// are a rotation, scaling and translation. The lumped transform is created
	/// with only 5+N multiplies versus 2(N^3) if using matrix multiplication.
	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat<N,T> TSR(const Vec<N-1,T>& t, const Vec<N-1,T>& s, const Rotoscale<T>& r){
		Mat m = SR<Dim1,Dim2>(s, r);
		// We have SR and need to compute TSR
		auto& c = m.col<N-1>().template sub<-1>();
		// Compute contribution of diagonal components
		for(int i=0; i<N-1; ++i) c[i] = m(i,i) * t[i];
		// Compute contribution of off-diagonal components
		c.template at<Dim1>() += m.at<Dim1,Dim2>() * t.template at<Dim2>();
		c.template at<Dim2>() += m.at<Dim2,Dim1>() * t.template at<Dim1>();
		return m;
	}

	/// Get translation-scaling-rotation-translation (TSRT) transform matrix

	/// This returns the transform matrix T*R*S*T where the respective matrices
	/// are a translation, rotation, scaling and translation. The lumped 
	/// transform is created with only 5+N multiplies versus 3(N^3) if using
	/// matrix multiplication.
	template <unsigned Dim1=0, unsigned Dim2=1>
	static Mat<N,T> TSRT(const Vec<N-1,T>& t1, const Vec<N-1,T>& s, const Rotoscale<T>& r, const Vec<N-1,T>& t2){
		Mat m = TSR<Dim1,Dim2>(t1, s, r);
		m.col<N-1>().template sub<-1>() += t2;
		return m;
	}

	//--------------------------------------------------------------------------
	// Access/Memory Operations

	/// Get temp copy
	Mat dup() const { return *this; }

	/// Returns C array type punned into a matrix
	static Mat& pun(T * src){ return *(Mat*)(src); }
	static const Mat& pun(const T * src){ return pun(const_cast<T*>(src)); }

	template <class UniformPOD>
	static Mat& pun(UniformPOD& v){
		static_assert(sizeof(UniformPOD) == sizeof(Mat), "Size mismatch");
		static_assert(!std::is_polymorphic<UniformPOD>::value, "Punning polymorphic class disallowed");
		return pun((T*)(&v));
	}

	template <class UniformPOD>
	static Mat& pun(const UniformPOD& v){ return pun(const_cast<UniformPOD&>(v)); }

	/// Returns order (number of columns or rows)
	static constexpr int order(){ return N; }

	/// Returns total number of elements
	static constexpr int size(){ return N*N; }

	/// Get read-only pointer to elements
	const T* elems() const { return mElems; }

	/// Get read-write pointer to elements
	T* elems(){ return mElems; }

	/// Set element at index with no bounds checking
	T& operator[](int i){ return mElems[i];}

	/// Get element at index with no bounds checking
	const T& operator[](int i) const { return mElems[i]; }

	/// Set element at row i, column j
	T& operator()(int i, int j){ return at(i,j); }

	/// Get element at row i, column j
	const T& operator()(int i, int j) const { return at(i,j); }

	/// Set element at row i, column j
	T& at(int i, int j){ return mElems[j*N+i]; }

	/// Get element at row i, column j
	const T& at(int i, int j) const { return mElems[j*N+i]; }

	template <int Row, int Col>
	T& at(){
		static_assert(Row<N && Col<N, "Row or column out of bounds");
		constexpr auto idx = Col*N+Row;
		return mElems[idx];
	}

	template <int Row, int Col>
	const T& at() const {
		return const_cast<Mat*>(this)->at<Row,Col>();
	}

	/// Get column as vector
	const Vec<N,T>& col(int i) const { return Vec<N,T>::pun(elems() + i*N); }
	Vec<N,T>& col(int i){ return Vec<N,T>::pun(elems() + i*N); }

	template <int Index>
	static constexpr int toSafeIndex(){
		constexpr int i = Index>=0 ? Index : N+Index;
		static_assert(0 <= i && i < N, "Index out of bounds");
		return i;
	}

	/// Get column as vector

	/// \tparam Index	Column index. If negative, uses N+Index.
	///
	template <int Index>
	Vec<N,T>& col(){
		constexpr auto j = toSafeIndex<Index>()*N;
		return Vec<N,T>::pun(elems() + j);
	}
	template <int Index>
	const Vec<N,T>& col() const {
		return const_cast<Mat*>(this)->col<Index>();
	}

	/// Get row as vector
	Vec<N,T> row(int i) const { return Vec<N,T>(elems()+i, N); }

	/// Get row as vector

	/// \tparam Index	Row index. If negative, uses N+Index.
	///
	template <int Index>
	Vec<N,T> row() const {
		return row(toSafeIndex<Index>());
	}

	/// Set row values
	template <class V>
	Mat& row(int i, const Vec<N,V>& v){
		for(int c=0; c<N; ++c) (*this)(i,c) = v[c];
		return *this;
	}

	/// Set row values

	/// \tparam Index	Row index. If negative, uses N+Index.
	/// \tparam V		Type of source values
	template <int Index, class V>
	Mat& row(const Vec<N,V>& v){
		return row(toSafeIndex<Index>(), v);
	}

	/// Return diagonal
	Vec<N,T> diagonal() const { return Vec<N,T>(elems(), N+1); }

	/// Transpose elements

	/// \param[in] size		Size of upper-left submatrix to transpose
	///
	Mat& transpose(int size=N){
		for(int j=0; j<size-1; ++j){	// row and column
		for(int i=j+1; i<size; ++i){	// offset into row or column
			std::swap((*this)(i,j), (*this)(j,i));
		}}
		return *this;
	}

	template <int M>
	Mat& transpose(){
		static_assert(0 <= M && M <= N, "Invalid (sub)matrix size");
		return transpose(M);
	}

	/// Get transposed copy
	Mat transposed(int size=N) const { return Mat(*this).transpose(size); }

	template <int M>
	Mat transposed() const { return Mat(*this).transpose<M>(); }

	/// Get an MxM submatrix
	template <int M>
	Mat<M,T> sub(int row=0, int col=0) const {
		static_assert(M<=N, "Submatrix size cannot be larger than matrix size");
		Mat<M,T> res(MAT_NO_INIT);
		for(int j=0; j<M; ++j){
		for(int i=0; i<M; ++i){
			res(j,i) = at(j+row, i+col);
		}}
		return res;
	}

	/// Returns a submatrix by removing one row and column
	Mat<N-1,T> submatrix(int row, int col) const {
		Mat<N-1,T> res(MAT_NO_INIT);
		for(int j=0,js=0; j<N-1; ++j, ++js){
			js += int(js==row);
			for(int i=0, is=0; i<N-1; ++i, ++is){
				is += int(is==col);
				res(j,i) = at(js,is);
			}
		}
		return res;
	}

	/// Swap two columns in place
	template <int Count=N> 
	Mat& swapCol(int dim1, int dim2){
		swap(
			col(dim1).template sub<Count>(),
			col(dim2).template sub<Count>()
		);
		return *this;
	}

	/// Swap two columns in place
	template <int Dim1, int Dim2, int Count=N> 
	Mat& swapCol(){
		static_assert_plane<Dim1,Dim2>();
		return swapCol<Count>(Dim1, Dim2);
	}

	/// Get reference to self as another type
	template <class V>
	V& as(){
		static_assert(sizeof(V) <= sizeof(*this), "Cannot pun to object of larger size");
		return *(V *)(elems());
	}

	template <class V>
	const V& as() const {
		return const_cast<Mat*>(this)->as<V>();
	}

	/// Return self punned as flat vector
	Vec<N*N,T>& vec(){ return as<Vec<N*N,T>>(); }

	/// Return self punned as vector of column vectors
	Vec<N,Vec<N,T>>& nest(){ return as<Vec<N,Vec<N,T>>>(); }


	/// Set elements from different sized matrix

	/// Only the corresponding elements are copied from the source. Extra
	/// elements in the destination are set according to the identity matrix.
	template <int M, class U>
	Mat& operator = (const Mat<M,U>& v){
		setIdentity(); // this could perhaps be done more efficiently
		constexpr auto L = N<M ? N : M;
		for(int r=0; r<L; ++r)
			for(int c=0; c<L; ++c)
				at(c,r) = v(c,r);
		return *this;
	}

	/// Set all elements to value
	Mat& set(const T& v){ IT(size()){ (*this)[i]=v; } return *this; }

	/// Set elements from another matrix
	template <class U>
	Mat& set(const Mat<N,U>& v){ return set(v.elems()); }

	/// Set elements in column-major order from C array
	template <class U>
	Mat& set(const U * arr){ IT(size()){ (*this)[i]=arr[i]; } return *this; }

	/// Set elements in column-major order from C array

	/// \param[in] arr			1D array from which to copy (stride=1)
	/// \param[in] numElements	number of elements to copy
	/// \param[in] matOffset	index offset into matrix
	/// \param[in] matStride	amount to stride through matrix
	template <class U>
	Mat& set(const U * arr, int numElements, int matOffset, int matStride=1){
		IT(numElements){ (*this)[i*matStride+matOffset]=arr[i]; } return *this;
	}

	/// Set 2-by-2 (sub)matrix from arguments
	Mat& set(
		const T& r1c1, const T& r1c2,
		const T& r2c1, const T& r2c2,
		int row=0, int col=0
	){
		setCol2(r1c1, r2c1, col  ,row);
		setCol2(r1c2, r2c2, col+1,row); return *this;
	}

	/// Set 3-by-3 (sub)matrix from arguments
	Mat& set(
		const T& r1c1, const T& r1c2, const T& r1c3,
		const T& r2c1, const T& r2c2, const T& r2c3,
		const T& r3c1, const T& r3c2, const T& r3c3,
		int row=0, int col=0
	){
		setCol3(r1c1, r2c1, r3c1, col  ,row);
		setCol3(r1c2, r2c2, r3c2, col+1,row);
		setCol3(r1c3, r2c3, r3c3, col+2,row); return *this;
	}

	/// Set 4-by-4 (sub)matrix from arguments
	Mat& set(
		const T& r1c1, const T& r1c2, const T& r1c3, const T& r1c4,
		const T& r2c1, const T& r2c2, const T& r2c3, const T& r2c4,
		const T& r3c1, const T& r3c2, const T& r3c3, const T& r3c4,
		const T& r4c1, const T& r4c2, const T& r4c3, const T& r4c4,
		int row=0, int col=0
	){
		setCol4(r1c1, r2c1, r3c1, r4c1, col  ,row);
		setCol4(r1c2, r2c2, r3c2, r4c2, col+1,row);
		setCol4(r1c3, r2c3, r3c3, r4c3, col+2,row);
		setCol4(r1c4, r2c4, r3c4, r4c4, col+3,row); return *this;
	}

	/// Set from column vectors
	template <class V>
	Mat& set(const Vec<2,V>& c1, const Vec<2,V>& c2){
		col<0>()=c1; col<1>()=c2;
		return *this;
	}
	/// Set from column vectors
	template <class V>
	Mat& set(const Vec<3,V>& c1, const Vec<3,V>& c2, const Vec<3,V>& c3){
		col<0>()=c1; col<1>()=c2; col<2>()=c3;
		return *this;
	}
	/// Set from column vectors
	template <class V>
	Mat& set(const Vec<4,V>& c1, const Vec<4,V>& c2, const Vec<4,V>& c3, const Vec<4,V>& c4){
		col<0>()=c1; col<1>()=c2; col<2>()=c3; col<3>()=c4;
		return *this;
	}

	/// Set a (sub)column
	Mat& setCol2(const T& v1, const T& v2, int col=0, int row=0){
		static_assert(N>=2, "Attempt to set matrix elements out of bounds");
		at(row  , col) = v1;
		at(row+1, col) = v2; return *this;
	}

	/// Set a (sub)column
	Mat& setCol3(const T& v1, const T& v2, const T& v3, int col=0, int row=0){
		static_assert(N>=3, "Attempt to set matrix elements out of bounds");
		at(row  , col) = v1;
		at(row+1, col) = v2;
		at(row+2, col) = v3; return *this;
	}

	/// Set a (sub)column
	Mat& setCol4(const T& v1, const T& v2, const T& v3, const T& v4, int col=0, int row=0){
		static_assert(N>=4, "Attempt to set matrix elements out of bounds");
		at(row,   col) = v1;
		at(row+1, col) = v2;
		at(row+2, col) = v3;
		at(row+3, col) = v4; return *this;
	}

	/// Set elements on diagonal to one and all others to zero
	Mat& setIdentity(){
		return diagonal(T(1));
	}

	/// Set elements on diagonal to a value and all others to zero
	Mat& diagonal(T v){
		for(int i=0; i<N; ++i)
			(*this)[i*(N+1)] = v;

		for(int i=0;   i<N-1  ; ++i){
		for(int j=i+1; j<N+i+1; ++j){
			(*this)[i*N + j] = T(0);
		}}
		return *this;
	}


	//--------------------------------------------------------------------------
	// Basic Arithmetic Operations

	bool operator == (const Mat& v) const {
		if(&v != this){
			IT(size()){
				if((*this)[i] != v[i]) return false;
			}
		}
		return true;
	}
	bool operator != (const Mat& v) const { return !(*this == v); }

	Mat& operator *= (const Mat& v){ return multiply(*this, Mat(*this),v); }
	Mat& operator += (const Mat& v){ IT(size()){ (*this)[i] += v[i]; } return *this; }
	Mat& operator -= (const Mat& v){ IT(size()){ (*this)[i] -= v[i]; } return *this; }
	Mat& operator += (const T& v){ IT(size()){ (*this)[i] += v; } return *this; }
	Mat& operator -= (const T& v){ IT(size()){ (*this)[i] -= v; } return *this; }
	Mat& operator *= (const T& v){ IT(size()){ (*this)[i] *= v; } return *this; }
	Mat& operator /= (const T& v){ IT(size()){ (*this)[i] /= v; } return *this; }

	Mat operator - () const { Mat r(MAT_NO_INIT); IT(size()){ r[i]=-(*this)[i]; } return r; }
	Mat operator + (const Mat& v) const { return Mat(*this) += v; }
	Mat operator - (const Mat& v) const { return Mat(*this) -= v; }
	Mat operator * (const Mat& v) const { return Mat(*this) *= v; }
	Mat operator + (const T& v) const { return Mat(*this) += v; }
	Mat operator - (const T& v) const { return Mat(*this) -= v; }
	Mat operator * (const T& v) const { return Mat(*this) *= v; }
	Mat operator / (const T& v) const { return Mat(*this) /= v; }

	/// Computes matrix product r = a * b

	/// Returns reference to result
	///
	static Mat& multiply(Mat& r, const Mat& a, const Mat& b){
		for(int j=0; j<N; ++j){
			const Vec<N,T>& bcol = b.col(j);
			for(int i=0; i<N; ++i){
				r(i,j) = a.row(i).dot(bcol);
			}
		}
		return r;
	}

	/// Computes product of matrix multiplied by column vector, r = m * vCol
	template <class U>
	static Vec<N,U>& multiply(Vec<N,U>& r, const Mat& m, const Vec<N,U>& vCol){
		IT(N){ r[i] = m.row(i).dot(vCol); }
		return r;
	}

	/// Computes product of row vector multiplied by matrix, r = vRow * m
	template <class U>
	static Vec<N,U>& multiply(Vec<N,U>& r, const Vec<N,U>& vRow, const Mat& m){
		IT(N){ r[i] = vRow.dot(m.col(i)); }
		return r;
	}

	/// Transform a point by this matrix treated as a homogeneous transform
	template <class U>
	Vec<N-1,U> transformPoint(const Vec<N-1,U>& v) const {
		Vec<N-1,U> r(VEC_NO_INIT);
		IT(N-1){
			auto mrow = row(i);
			r[i] = mrow.template sub<N-1>().dot(v) + mrow.back();
		}
		return r;
	}

	/// Transform a vector by this matrix treated as a homogeneous transform
	template <class U>
	Vec<N-1,U> transformVector(const Vec<N-1,U>& v) const {
		Vec<N-1,U> r(VEC_NO_INIT);
		IT(N-1){ r[i] = row(i).template sub<N-1>().dot(v); }
		return r;
	}


	//--------------------------------------------------------------------------
	// Linear Operations

	/// Returns inverse of matrix
	Mat<N,T> inverse() const {
		auto inv = *this;
		invert(inv);
		return inv;
	}

	/// Get cofactor
	T cofactor(int row, int col) const {
		T minor = determinant(submatrix(row,col));
		T cofactors[] = {minor, -minor};
		// cofactor sign: + if row+col even, - otherwise
		int sign = (row^col) & 1;
		return cofactors[sign];
	}

	/// Get cofactor matrix
	Mat<N,T> cofactorMatrix() const {
		Mat<N,T> res(MAT_NO_INIT);
		for(int r=0; r<N; ++r){
			for(int c=0; c<N; ++c){
				res(r,c) = cofactor(r,c);
			}
		}
		return res;
	}

	/// Get trace (sum of diagonal elements)
	T trace() const { return diagonal().sum(); }

	/// Extract rotation part of transformation matrix
	Mat<N-1,T> rotation() const {
		auto R = sub<N-1>();
		for(int i=0; i<N-1; ++i) R.col(i).normalize();
		return R;
	}

	/// Get Euler angle around local X axis (YXZ/AEB order), in radians
	T eulerX() const { return std::asin(-at<1,2>()); }

	/// Get Euler angle around local Y axis (YXZ/AEB order), in radians
	T eulerY() const { return std::atan2(at<0,2>(), at<2,2>()); }

	/// Get Euler angle around local Z axis (YXZ/AEB order), in radians
	T eulerZ() const { return std::atan2(at<1,0>(), at<1,1>()); }

	// Ref: https://www.geometrictools.com/Documentation/EulerAngles.pdf


	// Affine transformations

	/// Rotate transformation matrix on a local plane

	/// This efficiently computes A' = A*R with only 2(N-1) complex muls
	/// (or about 4(N-1) madds). The translation part is left unaffected.
	///
	/// \param[in] r		rotation (or angle in radians)
	/// \param[in] dim1		local coordinate frame axis to rotate away from
	/// \param[in] dim2		local coordinate frame axis to rotate towards
	Mat& rotate(const Rotoscale<T>& r, int dim1, int dim2){
		for(int R=0; R<N-1; ++R){
			auto &a = at(R, dim2), &b = at(R, dim1);
			auto t = Rotoscale<T>(a,b) * r;
			a = t.r; b = t.i;
		}
		return *this;
	}

	template <int Dim1=0, int Dim2=1>
	Mat& rotate(const Rotoscale<T>& r){
		static_assert_plane_homog<Dim1,Dim2>();
		return rotate(r, Dim1, Dim2);
	}

	/// Rotate by 90 degrees on local plane

	/// This very efficiently applies A' = A*R, where R is a rotation matrix of
	/// 90 degrees, with only a swap and negation.
	Mat& rotate90(int dim1, int dim2){
		auto& col1 = col(dim1).template sub<N-1>();
		auto& col2 = col(dim2).template sub<N-1>();
		swap(col1, col2);
		col2 = -col2;
		return *this;
	}

	template <int Dim1=0, int Dim2=1>
	Mat& rotate90(){
		static_assert_plane_homog<Dim1,Dim2>();
		return rotate90(Dim1, Dim2);
	}

	/// Rotate (submatrix) on a global plane

	/// This efficiently applies A' = R*A with only 2M complex muls (~4M madds).
	/// The translation part is affected unless applied to a submatrix.
	///
	/// \tparam M			size of submatrix
	/// \param[in] r		rotation (or angle in radians)
	/// \param[in] dim1		global axis to rotate away from
	/// \param[in] dim2		global axis to rotate towards
	template <int M=N>
	Mat& rotateGlobal(const Rotoscale<T>& r, int dim1, int dim2){
		static_assert(M<=N, "Invalid submatrix size");
		for(int C=0; C<M; ++C){
			auto &a = at(dim1, C), &b = at(dim2, C);
			auto t = r * Rotoscale<T>(a,b);
			a = t.r; b = t.i;
		}
		return *this;
	}

	template <int Dim1=0, int Dim2=1, int M=N>
	Mat& rotateGlobal(const Rotoscale<T>& r){
		static_assert_plane_homog<Dim1,Dim2>();
		return rotateGlobal<M>(r, Dim1, Dim2);
	}

	/// Scale transformation matrix

	/// This efficiently applies A' = A*S, where S is a non-uniform scaling
	/// matrix, in only (N-1)^2 madds. The translation part is never affected.
	template<class V>
	Mat& scale(const Vec<N-1,V>& amount){
		for(int C=0; C<N-1; ++C)
			col(C).template sub<N-1>() *= amount[C];
		return *this;
	}

	/// Scale transformation matrix by uniform amount
	template<class V>
	Mat& scale(const V& amount){ return scale(Vec<N-1,V>(amount)); }

	/// Scale transformation matrix
	template<typename... Vals>
	Mat& scale(Vals... vals){ return scale(Vec<(sizeof...(Vals)),T>(vals...)); }

	/// Scale transformation matrix global coordinates

	/// This efficiently applies A' = S*A, where S is a non-uniform scaling
	/// matrix, in only N(N-1) madds. The translation part is also scaled.
	template<class V>
	Mat& scaleGlobal(const Vec<N-1,V>& amount){
		for(int C=0; C<N; ++C)
			col(C).template sub<N-1>() *= amount;
		return *this;
	}

	template<typename... Vals>
	Mat& scaleGlobal(Vals... vals){ return scaleGlobal(Vec<(sizeof...(Vals)),T>(vals...)); }

	/// Translate transformation matrix in local coordinates

	/// This efficiently applies A' = A*T, where T is a translation matrix, in
	/// only (N-1)^2 madds. The rotation/scaling part is never affected.
	template<class V>
	Mat& translate(const Vec<N-1,V>& amount){
		for(int R=0; R<N-1; ++R)
			at(R,N-1) += row(R).template sub<N-1>().dot(amount);
		return *this;
	}

	/// Translate transformation matrix by same amount in all directions
	template<class V>
	Mat& translate(const V& amount){ return translate(Vec<N-1,V>(amount)); }

	/// Translate transformation matrix
	template<typename... Vals>
	Mat& translate(Vals... vals){ return translate(Vec<(sizeof...(Vals)),T>(vals...)); }

	/// Translate transformation matrix in global coordinates

	/// This efficiently applies A' = T*A, where T is a translation matrix, in
	/// only N-1 additions. The rotation/scaling part is never affected.
	template<class V>
	Mat& translateGlobal(const Vec<N-1,V>& amount){
		for(int R=0; R<N-1; ++R)
			at(R, N-1) += amount[R];
		return *this;
	}

	template<typename... Vals>
	Mat& translateGlobal(Vals... vals){ return translateGlobal(Vec<(sizeof...(Vals)),T>(vals...)); }

	/// Print to file (stream)
	void print(FILE * file = stdout) const;

private:
	template <int Dim1, int Dim2, int DimMax=N>
	static constexpr int static_assert_plane(){
		static_assert(Dim1<DimMax && Dim2<DimMax && Dim1!=Dim2, "Invalid plane dimensions");
		return {};
	}
	template <int Dim1, int Dim2>
	static constexpr int static_assert_plane_homog(){
		return static_assert_plane<Dim1,Dim2,N-1>();
	}
};


// -----------------------------------------------------------------------------
// The following are functions that either cannot be defined as class methods
// (due to syntax rules or specialization) or simply are not object oriented.

// Non-member binary arithmetic operations
template <int N, class T>
inline Mat<N,T> operator + (const T& s, const Mat<N,T>& v){ return  v+s; }

template <int N, class T>
inline Mat<N,T> operator - (const T& s, const Mat<N,T>& v){ return -v+s; }

template <int N, class T>
inline Mat<N,T> operator * (const T& s, const Mat<N,T>& v){ return  v*s; }


// Basic Vec/Mat Arithmetic
template <int N, class T, class U>
inline Vec<N,U> operator* (const Mat<N,T>& m, const Vec<N,U>& vCol){
	Vec<N,U> r; return Mat<N,T>::multiply(r, m,vCol);
}

template <int N, class T, class U>
inline Vec<N,U> operator* (const Vec<N,U>& vRow, const Mat<N,T>& m){
	Vec<N,U> r; return Mat<N,T>::multiply(r, vRow,m);
}

/// Get vector transform of row vector v by matrix A

/// This function can be used as a fast inverse transform A^-1 v if A consists
/// only of a rotation and translation, e.g. if A is a pose.
template <int N, class T, class U>
Vec<N-1,U> transformVector(const Vec<N-1,U>& v, const Mat<N,T>& A){
	Vec<N-1,U> r;
	IT(N-1){ r[i] = v.dot(A.col(i).template sub<N-1>()); }
	return r;
}

/// Get determinant

/// This computes the determinant using cofactor (or Laplace) expansion.
/// The algorithm operates by recursively computing determinants of submatrices.
/// For small matrices, optimized versions are used.
/// \ingroup allocore
template<int N, class T>
T determinant(const Mat<N,T>& m){
	T res = 0;
	for(int i=0; i<N; ++i){
		T entry = m(0,i);
		if(entry != T(0)){
			res += entry * m.cofactor(0,i);
		}
	}
	return res;
}

template <class T>
T determinant(const Mat<1,T>& m){
	return m(0,0);
}

template <class T>
T determinant(const Mat<2,T>& m){
	return m(0,0)*m(1,1) - m(0,1)*m(1,0);
}

template <class T>
T determinant(const Mat<3,T>& m){
	return
		m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) +
		m(0,1)*(m(1,2)*m(2,0) - m(1,0)*m(2,2)) +
		m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0));
}

/// Invert matrix, returns whether matrix was able to be inverted
/// For small matrices, optimized versions are used.
/// \ingroup allocore
template<int N, class T>
bool invert(Mat<N,T>& m){
	// Get cofactor matrix, C
	Mat<N,T> C = m.cofactorMatrix();

	// Compute determinant
	T det = T(0);
	for(int i=0; i<N; ++i){
		det += m(0,i) * C(0,i);
	}

	// Divide adjugate matrix, C^T, by determinant
	if(det != T(0)){
		m = (C.transpose() *= T(1)/det);
		return true;
	}

	return false;
}

template <class T>
bool invert(Mat<1,T>& m){
	T det = determinant(m);
	if(det != 0){
		m(0,0) /= det;
		return true;
	}
	return false;
}

template <class T>
bool invert(Mat<2,T>& m){
	T det = determinant(m);
	if(det != T(0)){
		m = Mat<2,T>(
			 m(1,1),-m(0,1),
			-m(1,0), m(0,0)
		) *= T(1)/det;
		return true;
	}
	return false;
}

template <class T>
bool invert(Mat<3,T>& m){
	auto nx = cross(m.template col<1>(), m.template col<2>());
	auto ny = cross(m.template col<2>(), m.template col<0>());
	auto nz = cross(m.template col<0>(), m.template col<1>());
	auto det= m.template at<0,0>()*nx.x + m.template at<1,0>()*ny.x + m.template at<2,0>()*nz.x;
	if(det != T(0)){
		m.set(
			nx.x, nx.y, nx.z,
			ny.x, ny.y, ny.z,
			nz.x, nz.y, nz.z
		) *= T(1)/det;
		return true;
	}
	return false;
}

/// Invert a proper rigid transformation matrix (rotation + translation)

/// This can be used to convert between camera and view matrices.
///
template <int N, class T>
void invertRigid(Mat<N,T>& m){
	// Given A = T * R, A^-1 = R^-1 * T^-1

	// Compute inverse translation T^-1
	T it[N-1];
	for(int i=0; i<N-1; ++i)
		it[i] = -(m.col(i).dot(m.template col<N-1>()));

	for(int r=0; r<N-1; ++r)
		m(r,N-1) = it[r];

	// Transpose rotation part to get R^-1
	m.transpose(N-1);
}

/// Get "normal" matrix from modelview

/// This matrix is used to convert normals from object to eye space.
/// It assumes the input matrix is composed of rotation, scaling, and
/// translation. It is normally obtained as the inverse transpose of the
/// modelview, however, this computes it much faster under the given assumption.
template <int N, class T> Mat<N-1,T> normalMatrix(const Mat<N,T>& mv){
	// Given A = S * R,
	//       N = (A^-1)^T
	//         = (R^-1 * S^-1)^T
	//         =  S^-1 * R

	// Compute S^-2
	T iss[N-1];
	for(int i=0; i<N-1; ++i)
		iss[i] = T(1)/sub<N-1>(mv.col(i)).magSqr();

	Mat<N-1,T> res = mv;

	// Apply S^-2
	for(int i=0; i<N-1; ++i)
		res.col(i) *= iss[i];

	return res;
}


/// Get rotation matrix that rotates one unit vector onto another

/// \param[in] from		Unit vector to rotate from
/// \param[in] to		Unit vector to rotate onto
template <class T>
Mat<3,T> rotation(const Vec<3,T>& from, const Vec<3,T>& to){
	// From https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
	auto cosAng = from.dot(to);

	if(cosAng == T(-1)){ // vectors point in opposite directions?
		return Mat<3,T>(T(-1)); // reflection
	}

	auto n = cross(from, to); // normal to plane of rotation

	// Skew-symmetric cross-product matrix: A^T = -A
	/*Mat<3,T> ssc(
		T(0),-n.z,  n.y,
		 n.z, T(0),-n.x,
		-n.y, n.x,  T(0)
	);
	return Mat<3,T>(T(1)) + ssc + ssc*ssc*(T(1)/(T(1)+cosAng));*/

	// Much faster version of above
	auto s = T(1)/(T(1)+cosAng);
	auto sx=s*n.x, sy=s*n.y, sz=s*n.z;
	auto sxx=sx*n.x, sxy=sx*n.y, szx=sx*n.z, syy=sy*n.y, syz=sy*n.z, szz=sz*n.z;
	return Mat<3,T>(
		T(1)-syy-szz, sxy-n.z, szx+n.y,
		sxy+n.z, T(1)-szz-sxx, syz-n.x,
		szx-n.y, syz+n.x, T(1)-sxx-syy
	);
}


//----------------------------
// Member function definitions

template <int N, class T>
void Mat<N,T>::print(FILE * file) const {
	for(int R=0; R<N; ++R){
		fprintf(file, "%c", " {"[R==0]);
		for(int C=0; C<N; ++C){
			fprintf(file, "% 6.3g%s",
				double((*this)(R,C)), (R!=N-1)||(C!=N-1) ? ", " : "}");
		}
		fprintf(file, "\n");
	}
}

template <int N, class T>
std::ostream& operator << (std::ostream& out, const Mat<N,T>& m) {
	for(int R=0; R<N; ++R){
		out << " {"[R==0];
		for(int C=0; C<N; ++C){
			out.width(6);
			out.precision(3);
			out << m(R,C);
			out << ((R!=N-1)||(C!=N-1) ? ", " : "}");
		}
		out << '\n';
	}
	return out;
}

#undef IT
} // al::
#endif
