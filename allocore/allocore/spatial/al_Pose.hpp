#ifndef INC_AL_POSE_HPP
#define INC_AL_POSE_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Representation of an oriented point by vector and quaternion

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Pablo Colapinto, 2010, wolftype@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include "allocore/math/al_Mat.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Quat.hpp"

namespace al {

/// @addtogroup allocore
/// @{

///	A local coordinate frame

///	A Pose is a combined position (3-vector) and orientation (quaternion).
/// Local coordinate bases are referred to as r, u, and f which stand for
class Pose {
public:

	/// @param[in] pos		Initial position
	/// @param[in] ori		Initial orientation
	Pose(const Vec3d& pos=Vec3d(0), const Quatd& ori=Quatd::identity());

	/// Construct from matrix
	template <class T>
	Pose(const Mat<4,T>& m){ fromMatrix(m); }

	/// Get identity
	static Pose identity(){ return Pose().setIdentity(); }


	// Arithmetic operations

	/// Get pose transformed by another pose
	Pose operator* (const Pose& v) const { return Pose(*this)*=v; }

	/// Translate and rotate by argument
	Pose& operator*= (const Pose& v){
		mPos += v.pos();
		mQuat*= v.quat();
		return *this;
	}

	/// Get pose rotated by a quaternion
	template <class T>
	Pose operator* (const Quat<T>& v) const { return Pose(*this)*=v; }

	/// Rotate pose by quaternion
	template <class T>
	Pose& operator*= (const Quat<T>& v){ mQuat*=v; return *this; }


	/// Turn to face a given world-coordinate point
	void faceToward(const Vec3d& p, double amt = 1.);

	/// Turn to face a given world-coordinate point, while maintaining an up vector
	void faceToward(const Vec3d& point, const Vec3d& up, double amt=1.);



	/// Get position
	Vec3d& pos(){ return mPos; }
	const Vec3d& pos() const { return mPos; }

	/// Get orientation (as a quaternion)
	Quatd& quat(){ return mQuat; }
	const Quatd& quat() const { return mQuat; }

	double x() const { return mPos.x; }
	double y() const { return mPos.y; }
	double z() const { return mPos.z; }

	/// Convert to 4x4 projection space matrix

	/// The first three columns hold the right, up and back vectors,
	/// respectively, according to a right-handed convention. The last column
	/// holds the position.
	Mat4d matrix() const;

	/// Convert to 4x4 direction matrix
	Mat4d directionMatrix() const;

	/// Get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d& to, double& azimuth, double& elevation, double& distance) const;


	/// Get world space X unit vector
	Vec3d ux() const { return mQuat.toVectorX(); }

	/// Get world space Y unit vector
	Vec3d uy() const { return mQuat.toVectorY(); }

	/// Get world space Z unit vector
	Vec3d uz() const { return mQuat.toVectorZ(); }

	/// Get world space unit vectors
	template <class T>
	void unitVectors(Vec<3,T>& ux, Vec<3,T>& uy, Vec<3,T>& uz) const {
		mQuat.toCoordinateFrame(ux, uy, uz);
	}

	/// Get local right, up, and forward unit vectors
	template <class T>
	void directionVectors(Vec<3,T>& ur, Vec<3,T>& uu, Vec<3,T>& uf) const {
		unitVectors(ur, uu, uf);
		uf = -uf;
	}

	/// Get right unit vector
	Vec3d ur() const { return ux(); }

	/// Get up unit vector
	Vec3d uu() const { return uy(); }

	/// Get forward unit vector (negative of Z)
	Vec3d uf() const { return -uz(); }

	// Overloaded cast operators
	operator Vec3d() const { return mPos; }
	operator Quatd() const { return mQuat; }

	/// Get a linear-interpolated Pose between this and another
	// (useful ingredient for smooth animations, estimations, etc.)
	Pose lerp(const Pose& target, double amt) const;


	// Setters

	/// Set to identity transform
	Pose& setIdentity();

	/// Set position
	template <class T>
	Pose& pos(const Vec<3,T>& v){ mPos = v; return *this; }

	/// Set position from individual components
	Pose& pos(double x, double y, double z) { return pos(Vec3d(x,y,z)); }

	/// Set quaternion component
	template <class T>
	Pose& quat(const Quat<T>& v){ mQuat = v; return *this; }

	/// Set orientation from Euler angles
	Pose& fromEuler(double azimuth, double elevation, double bank);

	template <class T>
	Pose& fromMatrix(const Mat<4,T>& v){
		mQuat.fromMatrix(v); // just reads upper 3x3
		mPos = v.template col<3>().xyz();
		return *this;
	}

	/// Print to standard output
	void print() const;

protected:
	Vec3d mPos;		// position in 3-space
	Quatd mQuat;	// orientation of reference frame as a quaternion (relative to global axes)
};



///	A mobile coordinate frame

///	This represents a Pose combined with smooth angular and positional
/// velocities. The smoothing is done using a one-pole low-pass filter which
/// produces an exponential ease-out type of transition.
class Nav : public Pose {
public:

	/// @param[in] pos		Initial position
	/// @param[in] smooth	Motion smoothing amount in [0,1)
	Nav(const Vec3d& pos = Vec3d(0), double smooth=0);


	/// Get smoothing amount
	double smooth() const { return mSmooth; }

	/// Get current linear and angular velocities as a Pose
	Pose vel() const;

	double velScale() const { return mVelScale; }


	/// Set smoothing amount in [0,1)
	Nav& smooth(double v){ mSmooth=v; return *this; }


	/// Set linear velocity
	Nav& move(double dr, double du, double df){ moveR(dr); moveU(du); return moveF(df); }
	template <class T>
	Nav& move(const Vec<3,T>& dp){ return move(dp.x, dp.y, dp.z); }

	/// Set linear velocity along right vector
	Nav& moveR(double v){ mMove0.x = v; return *this; }

	/// Set linear velocity along up vector
	Nav& moveU(double v){ mMove0.y = v; return *this; }

	/// Set linear velocity along forward vector
	Nav& moveF(double v){ mMove0.z = v; return *this; }

	Vec3d& move(){ return mMove0; }


	/// Move by a single increment
	Nav& nudge(double dr, double du, double df){ nudgeR(dr); nudgeU(du); return nudgeF(df); }
	template <class T>
	Nav& nudge(const Vec<3,T>& dp){ return nudge(dp.x, dp.y, dp.z); }

	Nav& nudgeR(double amount){ mNudge.x += amount; return *this; }
	Nav& nudgeU(double amount){ mNudge.y += amount; return *this; }
	Nav& nudgeF(double amount){ mNudge.z += amount; return *this; }

	/// Move toward a given world-coordinate point
	Nav& nudgeToward(const Vec3d& p, double amt=1.);

	Vec3d& nudge(){ return mNudge; }


	/// Set angular velocity from azimuth, elevation, and bank differentials, in radians
	Nav& spin(double da, double de, double db){ spinR(de); spinU(da); return spinF(db); }
	template <class T>
	Nav& spin(const Vec<3,T>& daeb){ return spin(daeb.x, daeb.y, daeb.z); }

	/// Set angular velocity from a unit quaternion (versor)
	Nav& spin(const Quatd& v){ v.toEuler(mSpin1); return *this; }

	/// Set angular velocity around right vector (elevation), in radians
	Nav& spinR(double v){ mSpin0.y = v; return *this; }

	/// Set angular velocity around up vector (azimuth), in radians
	Nav& spinU(double v){ mSpin0.x = v; return *this; }

	/// Set angular velocity around forward vector (bank), in radians
	Nav& spinF(double v){ mSpin0.z = v; return *this; }

	/// Set angular velocity directly
	Vec3d& spin(){ return mSpin0; }


	/// Turn by a single increment for one step, in radians
	Nav& turn(double az, double el, double ba){ turnR(el); turnU(az); return turnF(ba); }
	template <class T>
	Nav& turn(const Vec<3,T>& daeb){ return turn(daeb.x, daeb.y, daeb.z); }

	/// Turn by a single increment, in radians, around the right vector (elevation)
	Nav& turnR(double v){ mTurn.y = v; return *this;	}

	/// Turn by a single increment, in radians, around the up vector (azimuth)
	Nav& turnU(double v){ mTurn.x = v; return *this; }

	/// Turn by a single increment, in radians, around the forward vector (bank)
	Nav& turnF(double v){ mTurn.z = v; return *this; }

	Vec3d& turn(){ return mTurn; }


	/// Stop moving and spinning
	Nav& halt();

	/// Go to origin, reset orientation
	Nav& home();

	/// Finish any tweening (go directly to final target)
	Nav& finish();

	Nav& operator=(const Pose& v);

	/// Accumulate pose based on velocity
	void step(double dt=1);


	/// Get pull-back amount
	double pullBack() const { return mPullBack0; }

	/// Set pull-back amount
	Nav& pullBack(double v){ mPullBack0 = v>0. ? v : 0.; return *this; }

	/// Get transformed pose
	const Pose& transformed() const { return mTransformed; }
	Pose& transformed(){ return mTransformed; }

protected:
	Vec3d mMove0, mMove1;	// linear velocities (raw, smoothed)
	Vec3d mSpin0, mSpin1;	// angular velocities (raw, smoothed)
	Vec3d mTurn;			// orientation increment for one step
	Vec3d mNudge;			// position increment for one step
	double mSmooth;
	double mVelScale;		// velocity scaling factor
	double mPullBack0, mPullBack1;
	Pose mTransformed;
};

/// @} // end allocore group

} // al::
#endif
