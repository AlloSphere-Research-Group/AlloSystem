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
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Pablo Colapinto, 2010, wolftype@gmail.com
*/

#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Quat.hpp"

#include <stdio.h>


namespace al {

///	A local coordinate frame

///	A Pose is a combined position (3-vector) and orientation (quaternion).
/// Local coordinate bases are referred to as r, u, and f which stand for 
/// right, up, and forward, respectively.
class Pose {
public:
	/// Construct from a 
	Pose(const Vec3d& v=Vec3d(0), const Quatd& q=Quatd::identity())
	:	mVec(v), mQuat(q)
	{}
	
	Pose(const Pose& p) { set(p); }

	operator Vec3d() { return pos(); }
	operator Quatd() { return quat(); }


	// Arithmetic operations

	/// Get pose transformed by another pose
	Pose operator* (const Pose& v) const { return Pose(*this)*=v; }

	/// Translate and rotate by argument
	Pose& operator*= (const Pose& v){
		mVec += v.vec();
		mQuat*= v.quat();
		return *this;
	}


	/// Returns the identity
	static Pose identity(){ return Pose().setIdentity(); }


	/// Get "position" vector
	const Vec3d& pos() const { return mVec; }
	
	/// Get vector component
	const Vec3d& vec() const { return mVec; }

	/// Get quaternion component
	const Quatd& quat() const { return mQuat; }

	double x() const { return mVec[0]; }
	double y() const { return mVec[1]; }
	double z() const { return mVec[2]; }

	/// Convert to 4-by-4 projection space matrix
	Mat4d matrix() const {
		Mat4d m;
		quat().toMatrix(&m[0]);
		m.set(&vec()[0], 3, 12);
		return m;
	}

	Mat4d directionMatrix() const {
		Mat4d m = matrix();
		m(0,2) = -m(0,2);
		m(1,2) = -m(1,2);
		m(2,2) = -m(2,2);
		return m;
	}

	/// Get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d& to, double& azimuth, double& elevation, double& distance) const;

	/// Get world space unit vectors
	template <class T>
	void unitVectors(Vec<3,T>& ux, Vec<3,T>& uy, Vec<3,T>& uz) const {	
		quat().toVectorX(ux);
		quat().toVectorY(uy);
		quat().toVectorZ(uz);	
	}

	/// Get local right, up, and forward unit vectors
	template <class T>
	void directionVectors(Vec<3,T>& ur, Vec<3,T>& uu, Vec<3,T>& uf) const {
		unitVectors(ur, uu, uf);
		uf = -uf;
	}
	
	/// Get a linear-interpolated Pose between this and another
	// (useful ingredient for smooth animations, estimations, etc.)
	Pose lerp(Pose& target, double amt) const {
		Pose r(*this);
		r.pos().lerp(target.pos(), amt);
		r.quat().slerpTo(target.quat(), amt);
		return r;
	}
	
	/// Get right unit vector
	Vec3d ur() const { return ux(); }

	/// Get up unit vector
	Vec3d uu() const { return uy(); }

	/// Get forward unit vector (negative of Z)
	Vec3d uf() const { return -uz(); }
	
	/// Get X unit vector
	Vec3d ux() const { return quat().toVectorX(); }

	/// Get Y unit vector
	Vec3d uy() const { return quat().toVectorY(); }

	/// Get Z unit vector
	Vec3d uz() const { return quat().toVectorZ(); }


	// Setters

	Vec3d& pos(){ return mVec; }
	Vec3d& vec(){ return mVec; }
	Quatd& quat(){ return mQuat; }
	
	/// Copy all attributes from another Pose
	Pose& set(Pose& src){
		mVec = src.pos(); mQuat = src.quat(); return *this; }

	/// Set position
	template <class T>
	Pose& pos(const Vec<3,T>& v){ return vec(v); }
	
	/// Set position from individual components
	Pose& pos(double x, double y, double z) { return vec(Vec3d(x,y,z)); }

	/// Set vector component
	template <class T>
	Pose& vec(const Vec<3,T>& v){ mVec.set(v); return *this; }

	/// Set quaternion component
	template <class T>
	Pose& quat(const Quat<T>& v){ quat() = v; return *this; }

	/// Set state from another Pose
	Pose& set(const Pose& v){ mVec=v.vec(); mQuat=v.quat(); return *this; }

	/// Set to identity transform
	Pose& setIdentity(){ quat().setIdentity(); vec().set(0); return *this; }
	
	/// Print to standard output
	void print() const { printf("Vec3d(%f, %f, %f);\nQuatd(%f, %f, %f, %f);\n",
		mVec[0], mVec[1], mVec[2], mQuat[0], mQuat[1], mQuat[2], mQuat[3]); }

protected:
	Vec3d mVec;		// position in 3-space
	Quatd mQuat;	// orientation of reference frame as a quaternion (relative to global axes)
};



/// A smoothed Pose:
/// It approaches the stored target Pose exponentially
/// with a curvature determined by psmooth and qsmooth
class SmoothPose : public Pose {
public:
	SmoothPose(const Pose& init=Pose(), double psmooth=0.9, double qsmooth=0.9) 
	:	Pose(init), mTarget(init), mPF(psmooth), mQF(qsmooth) {}
	
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
	void target(const Pose& p) { mTarget.set(p); }
	void target(const Vec3d& p) { mTarget.pos().set(p); }
	void target(const Quatd& p) { mTarget.quat().set(p); } 
	
	// set immediately (without smoothing):
	void jump(Pose& p) {
		target(p);
		set(p);
	}
	void jump(Vec3d& v) {
		target(v);
		pos().set(v);
	}
	void jump(Quatd& q) {
		target(q);
		quat().set(q);
	}

protected:
	Pose mTarget;
	double mPF, mQF;
};



///	A mobile coordinate frame

///	A Pose that knows how to accumulate velocities
/// Smooth navigation with adjustable linear and angular velocity
class Nav : public Pose {
public:

	Nav(const Vec3d &position = Vec3d(0), double smooth=0)
	:	Pose(position), mSmooth(smooth), mVelScale(1)
	{	updateDirectionVectors(); }
	
	Nav(const Nav& nav)
	:	Pose(nav.pos(), nav.quat() ), 
		mMove0(nav.mMove0), mMove1(nav.mMove1),	// linear velocities (raw, smoothed)
		mSpin0(nav.mSpin0), mSpin1(nav.mSpin1),	// angular velocities (raw, smoothed)
		mTurn(nav.mTurn), mNudge(nav.mNudge),			//  
		mSmooth(nav.smooth()), mVelScale(nav.mVelScale)
	{	updateDirectionVectors(); }

	/// Get smoothing amount
	double smooth() const { return mSmooth; }

	/// Get right unit vector
	const Vec3d& ur() const { return mUR; }
	
	/// Get up unit vector
	const Vec3d& uu() const { return mUU; }
	
	/// Get forward unit vector
	const Vec3d& uf() const { return mUF; }
	
	/// Get linear and angular velocities as a Pose
	Pose vel() const {
		return Pose(mMove1, Quatd().fromEuler(mSpin1[1], mSpin1[0], mSpin1[2]));
	}
	
	double velScale() const { return mVelScale; }

	/// Set smoothing amount [0,1)
	Nav& smooth(double v){ mSmooth=v; return *this; }
	
	void view(double azimuth, double elevation, double bank) {
		view(Quatd().fromEuler(azimuth, elevation, bank));
	}
	void view(const Quatd& v) {
		quat(v);
		updateDirectionVectors();
	}
	
	void turn(const Quatd& v) {
		v.toEuler(mSpin1);
	}
	
	/// turn to face a given world-coordinate point 
	void faceToward(const Vec3d& p, double amt=1.) {
		Vec3d rotEuler;
		Vec3d target(p - pos());
		target.normalize();
		Quatd rot = Quatd::getRotationTo(uf(), target);
		rot.toEuler(rotEuler);
		mTurn.set(rotEuler * amt);
	}
	
	/// move toward a given world-coordinate point 
	void nudgeToward(const Vec3d& p, double amt=1.) {
		Vec3d rotEuler;
		Vec3d target(p - pos());
		target.normalize();	// unit vector of direction to move (in world frame)
		// rotate target into local frame:
		target = quat().rotate(target);
		// push ourselves in that particular direction:
		nudge(target * amt);
	}

	/// Set linear velocity
	void move(double dr, double du, double df) { moveR(dr); moveU(du); moveF(df); }
	void move(Vec3d dp) { moveR(dp[0]); moveU(dp[1]); moveF(dp[2]); }
	void moveR(double v){ mMove0[0] = v; }
	void moveU(double v){ mMove0[1] = v; }
	void moveF(double v){ mMove0[2] = v; }
	Vec3d& move() { return mMove0; }

	/// Accelerate
	void nudge(double ddr, double ddu, double ddf) { nudgeR(ddr); nudgeU(ddu); nudgeF(ddf); }
	void nudge(Vec3d dp) { nudgeR(dp[0]); nudgeU(dp[1]); nudgeF(dp[2]); }
	void nudgeR(double amount) { mNudge[0] += amount; }
	void nudgeU(double amount) { mNudge[1] += amount; }
	void nudgeF(double amount) { mNudge[2] += amount; }

	/// Set all angular velocity values from azimuth, elevation, and bank differentials, in radians
	void spin(double da, double de, double db){ spinR(de); spinU(da); spinF(db); }

	/// Set angular velocity around right vector, in radians
	void spinR(double v){ mSpin0[0] = v; }
	
	/// Set angular velocity around up vector, in radians
	void spinU(double v){ mSpin0[1] = v; }
	
	/// Set angular velocity around forward vector, in radians
	void spinF(double v){ mSpin0[2] = v; }
	
	/// Set angular velocity directly:
	Vec3d& spin() { return mSpin0; }

	/// Turn by a single increment for one step, in radians
	void turn(double a, double e, double b){ turnR(e); turnU(a); turnF(b); }
	void turnR(double v){ mTurn[0] = v; }
	void turnU(double v){ mTurn[1] = v; }
	void turnF(double v){ mTurn[2] = v; }

	/// Stop moving and spinning
	Nav& halt(){ 
		mMove0.set(0); 
		mMove1.set(0); 
		mSpin0.set(0); 
		mSpin1.set(0); 
		mTurn.set(0);
		mNudge.set(0);
		updateDirectionVectors();
		return *this; 
	}

	/// Go to origin, reset orientation
	Nav& home(){ 
		quat().identity();
		view(0, 0, 0);
		turn(0, 0, 0); 
		spin(0, 0, 0);
		vec().set(0);
		updateDirectionVectors();
		return *this;
	}

	/// Update coordinate frame basis vectors based on internal quaternion
	void updateDirectionVectors(){ 
		quat().normalize();
		directionVectors(mUR, mUU, mUF); 
	}
	
	void set(const Pose& v){
		Pose::set(v);
		updateDirectionVectors();
	}
	
	void set(const Nav& v){
		Pose::set(v);
		mMove0 = v.mMove0; mMove1 = v.mMove1;
		mSpin0 = v.mSpin0; mSpin1 = v.mSpin1;
		mTurn = v.mTurn;
		mUR = v.mUR; mUU = v.mUU; mUF = v.mUF;
		mSmooth = v.mSmooth;
		updateDirectionVectors();
	}
	
	/// Accumulate pose based on velocity
	void step(double dt=1){
		mVelScale = dt;
	
		double amt = 1.-smooth();	// TODO: adjust for dt

		//Vec3d angVel = mSpin0 + mTurn;

		// low-pass filter velocities
		mMove1.lerp(mMove0*dt + mNudge, amt);
		mSpin1.lerp(mSpin0*dt + mTurn, amt);

		// turn and nudge are a one-shot increments, so clear each frame
		mTurn.set(0);
		mNudge.set(0);

		mQuat *= vel().quat();
		updateDirectionVectors();

		// accumulate position:
		for(int i=0; i<pos().size(); ++i){
			pos()[i] += mMove1.dot(Vec3d(ur()[i], uu()[i], uf()[i]));
		}
	}

protected:
	Vec3d mMove0, mMove1;	// linear velocities (raw, smoothed)
	Vec3d mSpin0, mSpin1;	// angular velocities (raw, smoothed)
	Vec3d mTurn;
	Vec3d mNudge;			//  
	Vec3d mUR, mUU, mUF;	// basis vectors of local coordinate frame
	double mSmooth;
	double mVelScale;		// velocity scaling factor
};




// Implementation --------------------------------------------------------------



} // al::

#endif
