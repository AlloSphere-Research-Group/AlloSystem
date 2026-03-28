#ifndef INC_AL_INTERVAL_HPP
#define INC_AL_INTERVAL_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	A closed interval

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

namespace al {

/// A closed interval [min, max]

/// An interval is a connected region of the real line. Geometrically, it
/// describes a 0-sphere. Order is strongly enforced so that the endpoints will
/// always satisfy min <= max.
///
/// \ingroup allocore
template <class T>
class Interval{
public:

	using value_type = T;

	Interval()
	:	mMin(0), mMax(1){}

	/// \param[in] min	minimum endpoint
	/// \param[in] max	maximum endpoint
	Interval(const T& min, const T& max)
	{ endpoints(min,max); }

	T diameter() const { return mMax-mMin; }		///< Returns absolute difference of endpoints
	T size() const { return diameter(); }			///< Returns absolute difference of endpoints
	T radius() const { return diameter()/T(2); }	///< Returns one-half the diameter
	const T& max() const { return mMax; }			///< Get maximum endpoint
	const T& min() const { return mMin; }			///< Get minimum endpoint
	T center() const { return (mMax+mMin)/T(2); }	///< Returns center point
	bool proper() const { return mMin!=mMax; }		///< Returns true if diameter is non-zero
	bool degenerate() const { return mMin==mMax; }	///< Returns true if diameter is zero

	/// Get absolute value of interval
	Interval abs() const { return {abs(mMin), abs(mMax)}; }

	/// Returns true if value is in interval
	bool contains(const T& v) const { return v>=mMin && v<=mMax; }

	/// Linearly map point in interval to point in the unit interval [0,1]
	T toUnit(const T& v) const { return proper() ? toUnitUnsafe(v) : T(0); }
	T toUnitUnsafe(const T& v) const { return (v-mMin)/diameter(); }

	/// Linearly map point in the unit interval [0,1] to point in interval
	T fromUnit(const T& u) const { return u*diameter() + mMin; }

	template <class U>
	bool operator == (const Interval<U>& v){ return mMin==v.mMin && mMax==v.mMax; }

	template <class U>
	bool operator != (const Interval<U>& v){ return !(*this == v); }

	template <class U>
	Interval& operator +=(const Interval<U>& v){ endpoints(mMin+v.mMin, mMax+v.mMax); return *this; }

	template <class U>
	Interval& operator -=(const Interval<U>& v){ endpoints(mMin-v.mMax, mMax-v.mMin); return *this; }

	template <class U>
	Interval& operator *=(const Interval<U>& v){
		T a=mMin*v.mMin, b=mMin*v.mMax, c=mMax*v.mMin, d=mMax*v.mMax;
		return set(
			min(min(a,b),min(c,d)),
			max(max(a,b),max(c,d))
		);
	}

	template <class U>
	Interval& operator /=(const Interval<U>& v){
		T a=mMin/v.mMin, b=mMin/v.mMax, c=mMax/v.mMin, d=mMax/v.mMax;
		return set(
			min(min(a,b),min(c,d)),
			max(max(a,b),max(c,d))
		);
	}

	/// Set center point preserving diameter
	Interval& center(const T& v){ return centerDiameter(v, diameter()); }

	/// Set diameter (width) preserving center
	Interval& diameter(const T& v){ return centerDiameter(center(), v); }

	/// Set center and diameter
	Interval& centerDiameter(const T& c, const T& d){
		mMin = c - d*T(0.5);
		mMax = mMin + d;
		return *this;
	}

	/// Set the endpoints
	Interval& endpoints(const T& min, const T& max){
		return min <= max ? set(min,max) : set(max,min);
	}

	/// Translate interval by fixed amount
	Interval& translate(const T& v){ mMin+=v; mMax+=v; return *this; }

	/// Set maximum endpoint
	Interval& max(const T& v){ return endpoints(mMin, v); }

	/// Set minimum endpoint
	Interval& min(const T& v){ return endpoints(v, mMax); }

	Interval& operator=(const Interval& v){ return set(v.mMin, v.mMax); }

	template <class V>
	Interval& operator=(const V& v){ return set(v,v); }

	/// Initialize interval for fitting to input values
	Interval& resetForFitting(T extrema = 3e38){
		return set(extrema, -extrema);
	}

	/// Adjust interval to include value
	Interval& adjust(const T& v){
		if(v < mMin) mMin = v;
		if(v > mMax) mMax = v;
		return *this;
	}

private:
	T mMin, mMax;

	// Helper setter, no validation checks, pass-by-value to avoid self-referencing
	Interval& set(T min, T max){
		mMin=min; mMax=max; return *this;
	}

	static const T& min(const T& a, const T& b){ return a<b?a:b; }
	static const T& max(const T& a, const T& b){ return a>b?a:b; }
	static T abs(const T& x){ return x>=T(0)?x:-x; }
};

} // al::
#endif
