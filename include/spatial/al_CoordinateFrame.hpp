#ifndef INCLUDE_AL_COORDINATEFRAME_HPP
#define INCLUDE_AL_COORDINATEFRAME_HPP

#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"


namespace al {

///<	A coordinate frame
///		Combines a Vec3d position with a Quat orientation
class Pose {
public:



	///	position in 3-space
	Vec3d mPos;
	/// orientation of reference frame as a quaternion (relative to global axes)
	Quatd mQuat;
	
	Pose() : mPos(0) {}
	
	// TODO: conversion operators for Pose->Vec3d, Pose->Quatd?
	
protected:
};

///<	A mobile coordinate frame
///		A Pose that knows how to accumulate velocities
class Nav : public Pose {
public:
	///	d.pos for spatial component
	/// d.q for angular component
	Pose mVel;

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
		mVel.mPos *= amt;
		mVel.mQuat *= amt;
		updateUnitVectors();
	}
	
	void	view(const Quatd & v) { mQuat.set(v); updateUnitVectors(); }
	void	turn(const Quatd & v) { mVel.mQuat.set(v); }
	
	void	move(double x, double y, double z) { moveX(x); moveY(y); moveZ(z); }
	void	moveX(double amount) { mVel.mPos[0] = amount; }
	void	moveY(double amount) { mVel.mPos[1] = amount; }
	void	moveZ(double amount) { mVel.mPos[2] = amount; }
	
	void	push(double x, double y, double z) { pushX(x); pushY(y); pushZ(z); }
	void	pushX(double amount) { mVel.mPos[0] += amount; }
	void	pushY(double amount) { mVel.mPos[1] += amount; }
	void	pushZ(double amount) { mVel.mPos[2] += amount; }
	
	void	halt() { mVel.mQuat.identity(); mVel.mPos.set(0); }
	void	home() { mQuat.identity(); mPos.set(0); updateUnitVectors(); }
	
	// get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d & to, double azimuth, double elevation, double distance);
	
protected:
	// unit vectors for the current pose
	Vec3d mUX, mUY, mUZ;
};



class NavNode : public Nav {
public:

	void parent(Nav * v){ mParent = v; }

	Nav * parent(){ return mParent; }

protected:
	Nav * mParent;
};


} // al::

#endif
