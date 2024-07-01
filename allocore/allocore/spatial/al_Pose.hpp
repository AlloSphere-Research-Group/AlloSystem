#ifndef INCLUDE_AL_POSE_HPP
#define INCLUDE_AL_POSE_HPP

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
	Representing an oriented point by vector and quaternion

	File author(s):
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
		mVec += v.vec();
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



	/// Get "position" vector
	Vec3d& pos(){ return mVec; }
	const Vec3d& pos() const { return mVec; }

	/// Get vector component
	Vec3d& vec(){ return mVec; }
	const Vec3d& vec() const { return mVec; }

	/// Get quaternion component (represents orientation)
	Quatd& quat(){ return mQuat; }
	const Quatd& quat() const { return mQuat; }

	double x() const { return mVec[0]; }
	double y() const { return mVec[1]; }
	double z() const { return mVec[2]; }

	/// Convert to 4x4 projection space matrix
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
	operator Vec3d() const { return mVec; }
	operator Quatd() const { return mQuat; }

	/// Get a linear-interpolated Pose between this and another
	// (useful ingredient for smooth animations, estimations, etc.)
	Pose lerp(const Pose& target, double amt) const;


	// Setters

	/// Set to identity transform
	Pose& setIdentity();

	/// Set position
	template <class T>
	Pose& pos(const Vec<3,T>& v){ return vec(v); }

	/// Set position from individual components
	Pose& pos(double x, double y, double z) { return vec(Vec3d(x,y,z)); }

	/// Set vector component
	template <class T>
	Pose& vec(const Vec<3,T>& v){ mVec = v; return *this; }

	/// Set quaternion component
	template <class T>
	Pose& quat(const Quat<T>& v){ mQuat = v; return *this; }

	/// Set orientation from Euler angles
	Pose& fromEuler(double azimuth, double elevation, double bank);

	template <class T>
	Pose& fromMatrix(const Mat<4,T>& v){
		mQuat.fromMatrix(v); // just reads upper 3x3
		mVec = v.template col<3>().xyz();
		return *this;
	}

	/// Print to standard output
	void print() const;

protected:
	Vec3d mVec;		// position in 3-space
	Quatd mQuat;	// orientation of reference frame as a quaternion (relative to global axes)
};



/// A smoothed Pose

/// This Pose approaches the stored target Pose exponentially
/// with a curvature determined by psmooth and qsmooth
class SmoothPose : public Pose {
public:
	SmoothPose(const Pose& init=Pose(), double psmooth=0.9, double qsmooth=0.9);

	// step toward the target:
	SmoothPose& operator()() {
		pos().lerp(mTarget.pos(), 1.-mPF);
		quat().slerpTo(mTarget.quat(), 1.-mQF);
		return *this;
	}

	// set and update:
	SmoothPose& operator()(const Pose& p) {
		target(p);
		return (*this)();
	}

	// set the target to smoothly interpolate to:
	Pose& target() { return mTarget; }
	void target(const Pose& v) { mTarget = v; }
	void target(const Vec3d& v) { mTarget.pos() = v; }
	void target(const Quatd& v) { mTarget.quat() = v; }

	// set immediately (without smoothing):
	void jump(Pose& p) {
		target(p);
		*this = p;
	}
	void jump(Vec3d& v) {
		target(v);
		pos() = v;
	}
	void jump(Quatd& q) {
		target(q);
		quat() = q;
	}

protected:
	Pose mTarget;
	double mPF, mQF;
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

	/// Copy constructor
	Nav(const Nav& nav);


	/// Get smoothing amount
	double smooth() const { return mSmooth; }

	/// Get current linear and angular velocities as a Pose
	Pose vel() const;

	double velScale() const { return mVelScale; }


	/// Set smoothing amount in [0,1)
	Nav& smooth(double v){ mSmooth=v; return *this; }


	/// Set linear velocity
	void move(double dr, double du, double df){ moveR(dr); moveU(du); moveF(df); }
	template <class T>
	void move(const Vec<3,T>& dp){ move(dp[0],dp[1],dp[2]); }

	/// Set linear velocity along right vector
	void moveR(double v){ mMove0[0] = v; }

	/// Set linear velocity along up vector
	void moveU(double v){ mMove0[1] = v; }

	/// Set linear velocity along forward vector
	void moveF(double v){ mMove0[2] = v; }

	Vec3d& move(){ return mMove0; }


	/// Move by a single increment
	void nudge(double dr, double du, double df){ nudgeR(dr); nudgeU(du); nudgeF(df); }
	template <class T>
	void nudge(const Vec<3,T>& dp){ nudge(dp[0],dp[1],dp[2]); }

	void nudgeR(double amount){ mNudge[0] += amount; }
	void nudgeU(double amount){ mNudge[1] += amount; }
	void nudgeF(double amount){ mNudge[2] += amount; }

	/// Move toward a given world-coordinate point
	void nudgeToward(const Vec3d& p, double amt=1.);

	Vec3d& nudge(){ return mNudge; }


	/// Set angular velocity from azimuth, elevation, and bank differentials, in radians
	void spin(double da, double de, double db){ spinR(de); spinU(da); spinF(db); }
	template <class T>
	void spin(const Vec<3,T>& daeb){ spin(daeb[0],daeb[1],daeb[2]); }

	/// Set angular velocity from a unit quaternion (versor)
	void spin(const Quatd& v){ v.toEuler(mSpin1); }

	/// Set angular velocity around right vector (elevation), in radians
	void spinR(double v){ mSpin0[1] = v; }

	/// Set angular velocity around up vector (azimuth), in radians
	void spinU(double v){ mSpin0[0] = v; }

	/// Set angular velocity around forward vector (bank), in radians
	void spinF(double v){ mSpin0[2] = v; }

	/// Set angular velocity directly
	Vec3d& spin(){ return mSpin0; }


	/// Turn by a single increment for one step, in radians
	void turn(double az, double el, double ba){ turnR(el); turnU(az); turnF(ba); }
	template <class T>
	void turn(const Vec<3,T>& daeb){ turn(daeb[0],daeb[1],daeb[2]); }

	/// Turn by a single increment, in radians, around the right vector (elevation)
	void turnR(double v){ mTurn[1] = v;	}

	/// Turn by a single increment, in radians, around the up vector (azimuth)
	void turnU(double v){ mTurn[0] = v; }

	/// Turn by a single increment, in radians, around the forward vector (bank)
	void turnF(double v){ mTurn[2] = v; }

	Vec3d& turn(){ return mTurn; }


	/// Stop moving and spinning
	Nav& halt();

	/// Go to origin, reset orientation
	Nav& home();

	Nav& operator=(const Pose& v);

	Nav& operator=(const Nav& v);

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
