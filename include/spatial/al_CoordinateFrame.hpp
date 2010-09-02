#ifndef INCLUDE_AL_COORDINATEFRAME_HPP
#define INCLUDE_AL_COORDINATEFRAME_HPP

#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"


namespace al {

///<	A coordinate frame
///		Combines a Vec3d position with a Quat orientation
class Pose {
public:
	Pose(const Vec3d &v=Vec3d(0)): mVec(v) { mQuat.identity(); }
	
	Vec3d& pos(){ return mVec; }
	const Vec3d& pos() const { return mVec; }
	
	template <class T>
	Pose& pos(const Vec3<T>& v){ return vec(v); }
	
	Vec3d& vec(){ return mVec; }
	const Vec3d& vec() const { return mVec; }

	template <class T>
	Pose& vec(const Vec3<T>& v){ mVec.set(v); return *this; }

	Quatd& quat(){ return mQuat; }
	const Quatd& quat() const { return mQuat; }

	template <class T>
	Pose& quat(const Quat<T>& v){
		mQuat[0]=v[0]; mQuat[1]=v[1]; mQuat[2]=v[2]; mQuat[3]=v[3];
		return *this;
	}

	// TODO: conversion operators for Pose->Vec3d, Pose->Quatd?	
protected:
	Vec3d mVec;		// position in 3-space
	Quatd mQuat;	// orientation of reference frame as a quaternion (relative to global axes)
};


///<	A mobile coordinate frame
///		A Pose that knows how to accumulate velocities
class Nav : public Pose {
public:

	Nav(const Vec3d &v = Vec3d(0));

	const Vec3d& ux() const { return mUX; }
	const Vec3d& uy() const { return mUY; }
	const Vec3d& uz() const { return mUZ; }
	Pose& vel(){ return mVel; }
	const Pose& vel() const { return mVel; }
	
	/// Update coordinate frame basis vectors based on internal quaternion
	void updateUnitVectors();
	
	/// accumulate pose based on velocity
	void step(double dt);
	/// accumulate pose based on velocity (faster path for dt == 1)
	void step();
	
	/// scale the velocities by amt:
	void decay(double amt);

	void view(const Quatd & v);
	void turn(const Quatd & v);

	void move(double x, double y, double z) { moveX(x); moveY(y); moveZ(z); }
	void moveX(double amount) { vel().vec()[0] = amount; }
	void moveY(double amount) { vel().vec()[1] = amount; }
	void moveZ(double amount) { vel().vec()[2] = amount; }
	
	void push(double x, double y, double z) { pushX(x); pushY(y); pushZ(z); }
	void pushX(double amount) { vel().vec()[0] += amount; }
	void pushY(double amount) { vel().vec()[1] += amount; }
	void pushZ(double amount) { vel().vec()[2] += amount; }

	//void	rotateX(double amount) { vel().quat()
	
	void halt();
	void home();
	
	// get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d& to, double& azimuth, double& elevation, double& distance) const;
	
protected:
	Pose mVel;
	Vec3d mUX, mUY, mUZ; // unit vectors for the current pose
};



/// Smooth navigation with adjustable linear and angular velocity
class NavSmooth : public Pose {
public:

	NavSmooth(const Vec3d &position = Vec3d(0), double smooth=0.85, double turnRate=2)
	:	Pose(position), mSmooth(smooth)
	{	updateUnitVectors(); }

	/// Get smoothing amount
	double smooth() const { return mSmooth; }

	const Vec3d& ux() const { return mUX; }
	const Vec3d& uy() const { return mUY; }
	const Vec3d& uz() const { return mUZ; }
	
	/// Get linear and angular velocities as a Pose
	Pose vel() const {
		Pose p;
		p.vec(mMove1);
		p.quat(Quatd::fromEuler(mSpin1[1], mSpin1[0], mSpin1[2]));
		return p;
	}

	/// Set smoothing amount [0,1)
	NavSmooth& smooth(double v){ mSmooth=v; return *this; }

	/// Set linear velocity
	void move(double x, double y, double z) { moveX(x); moveY(y); moveZ(z); }
	void moveX(double v){ mMove0[0] = v; }
	void moveY(double v){ mMove0[1] = v; }
	void moveZ(double v){ mMove0[2] = v; }

	/// Accelerate
	void push(double x, double y, double z) { pushX(x); pushY(y); pushZ(z); }
	void pushX(double amount) { mMove0[0] += amount; }
	void pushY(double amount) { mMove0[1] += amount; }
	void pushZ(double amount) { mMove0[2] += amount; }

	/// Set angular velocity
	void spinX(double v){ mSpin0[0] = v; }
	void spinY(double v){ mSpin0[1] = v; }
	void spinZ(double v){ mSpin0[2] = v; }

	/// Turn by a single increment for one step
	void turnX(double v){ mTurn[0] = v; }
	void turnY(double v){ mTurn[1] = v; }
	void turnZ(double v){ mTurn[2] = v; }

	/// Stop moving and spinning
	NavSmooth& halt(){ mMove0.set(0); mSpin0.set(0); return *this; }

	/// Go to origin and reset orientation to identity
	NavSmooth& home(){ 
		quat().identity();
		vec().set(0);
		updateUnitVectors();
		return *this;
	}

	/// Update coordinate frame basis vectors based on internal quaternion
	void updateUnitVectors(){
		quat().normalize();
		quat().toVectorX(mUX);
		quat().toVectorY(mUY);
		quat().toVectorZ(mUZ);	
	}
	
	/// Accumulate pose based on velocity
	void step(double dt=1){
		double amt = 1.-smooth();	// TODO: adjust for dt

		Vec3d angVel = mSpin0 + mTurn;
		mTurn.set(0); // turn is just a one-shot increment, so clear each frame

		// low-pass filter velocities
		mMove1.lerp(mMove0*dt, amt);
		mSpin1.lerp(angVel*dt, amt);

		mQuat *= vel().quat();
		updateUnitVectors();

		// accumulate position:
		for(int i=0; i<pos().size(); ++i){
			pos()[i] += mMove1.dot(Vec3d(ux()[i], uy()[i], uz()[i]));
		}
	}

protected:
	Vec3d mMove0, mMove1;	// linear velocities (raw, smoothed)
	Vec3d mSpin0, mSpin1;	// angular velocities (raw, smoothed)
	Vec3d mTurn;			// 
	Vec3d mUX, mUY, mUZ;	// basis vectors of coordinate frame
	double mSmooth;
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


// Implementation --------------------------------------------------------------

inline void Nav :: updateUnitVectors() {
	mQuat.normalize();
	mQuat.toVectorX(mUX);
	mQuat.toVectorY(mUY);
	mQuat.toVectorZ(mUZ);
}

inline void Nav :: decay(double amt) {
	mVel.vec() *= amt;
	mVel.quat() *= amt;
	updateUnitVectors();
}

inline void Nav :: view(const Quatd & v) { 
	quat().set(v);
	updateUnitVectors();
}

inline void Nav :: turn(const Quatd & v) {
	vel().quat().set(v);
}

inline void Nav :: halt() {
	vel().quat().identity();
	vel().vec().set(0);
}

inline void Nav :: home() {
	quat().identity();
	vec().set(0);
	updateUnitVectors();
}


} // al::

#endif
