#ifndef INC_AL_ROTOSCALE_HPP
#define INC_AL_ROTOSCALE_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Combined rotation and uniform scaling transform

	Author(s):
	Lance Putnam, 2026
*/

#include <cmath> // cos, sin

namespace al {

/// Combined rotation and uniform scaling

/// This class provides a uniform way of representing a rotation and scaling.
/// It is essentially a complex number, z, where arg(z) is the rotation angle
/// and |z| is the scaling amount. The transform is stored in rectangular form
/// for efficiency reasons. When building rotation matrices, we typically need
/// to compute cos(angle) and sin(angle), so these are essentially cached here.
/// The scaling part is optional, but comes for free, so is included.
/// \ingroup allocore
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

using Rotoscalef = Rotoscale< float>;
using Rotoscaled = Rotoscale<double>;

} // al::
#endif
