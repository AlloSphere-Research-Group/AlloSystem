#ifndef INCLUDE_AL_COORDINATEFRAME_HPP
#define INCLUDE_AL_COORDINATEFRAME_HPP

#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"


namespace al {

///<	A coordinate frame
///		Combines a Vec3d position with a Quat orientation
class Pose {
public:
	Pose(): mVec(0){ mQuat.identity(); }

	Vec3d& vec(){ return mVec; }
	const Vec3d& vec() const { return mVec; }

	Quatd& quat(){ return mQuat; }
	const Quatd& quat() const { return mQuat; }

	// TODO: conversion operators for Pose->Vec3d, Pose->Quatd?	
protected:
	Vec3d mVec;		// position in 3-space
	Quatd mQuat;	// orientation of reference frame as a quaternion (relative to global axes)
};


///<	A mobile coordinate frame
///		A Pose that knows how to accumulate velocities
class Nav : public Pose {
public:

	Pose& vel(){ return mVel; }
	const Pose& vel() const { return mVel; }

	const Vec3d& ux() const { return mUX; }
	const Vec3d& uy() const { return mUY; }
	const Vec3d& uz() const { return mUZ; }
	
	/// set mUX, mUY, mUZ based on mQuat:
	void updateUnitVectors();
	
	/// accumulate pose based on velocity
	void step(double dt);
	/// accumulate pose based on velocity (faster path for dt == 1)
	void step();
	
	/// scale the velocities by amt:
	void decay(double amt) {
		mVel.vec() *= amt;
		mVel.quat() *= amt;
		updateUnitVectors();
	}
	
	void	view(const Quatd & v) { quat().set(v); updateUnitVectors(); }
	void	turn(const Quatd & v) { vel().quat().set(v); }
	
	void	move(double x, double y, double z) { moveX(x); moveY(y); moveZ(z); }
	void	moveX(double amount) { vel().vec()[0] = amount; }
	void	moveY(double amount) { vel().vec()[1] = amount; }
	void	moveZ(double amount) { vel().vec()[2] = amount; }
	
	void	push(double x, double y, double z) { pushX(x); pushY(y); pushZ(z); }
	void	pushX(double amount) { vel().vec()[0] += amount; }
	void	pushY(double amount) { vel().vec()[1] += amount; }
	void	pushZ(double amount) { vel().vec()[2] += amount; }
	
	void	halt() { vel().quat().identity(); vel().vec().set(0); }
	void	home() { quat().identity(); vec().set(0); updateUnitVectors(); }
	
	// get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d& to, double& azimuth, double& elevation, double& distance) const;
	
protected:
	Pose mVel;
	Vec3d mUX, mUY, mUZ; // unit vectors for the current pose
};



class NavRef : public Nav {
public:
	NavRef()
	: mParent(0)
	{}

	void parent(Nav * v){ mParent = v; }

	Nav * parent(){ return mParent; }

protected:
	Nav * mParent;
};


} // al::

#endif
