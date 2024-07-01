#include <cstdio>
#include "allocore/spatial/al_Pose.hpp"

namespace al{

Pose::Pose(const Vec3d& v, const Quatd& q)
:	mVec(v), mQuat(q)
{}

void Pose::faceToward(const Vec3d& point, double amt){

	auto target = (point - pos()).normalize();
	auto rot = Quatd::getRotationTo(uf(), target);

	// We must pre-multiply the Pose quaternion with our rotation since
	// it was computed in world space.
	if(amt == 1.)	quat() = rot * quat();
	else			quat() = rot.pow(amt) * quat();

	/* Apply rotation using Euler angles (can behave erraticly)
	Vec3d aeb1, aeb2;
	quat().toEuler(aeb1);
	(rot * quat()).toEuler(aeb2);
	Vec3d rotEuler = aeb2 - aeb1;
	//for(int i=0; i<3; ++i){ // minimize angles
	//	double& r = rotEuler[i];
	//	if(r > M_PI) r -= 2*M_PI;
	//	else if(r < -M_PI) r += 2*M_PI;
	//}
	rot.toEuler(rotEuler);
	mTurn.set(rotEuler * amt);
	//*/
}

void Pose::faceToward(const Vec3d& point, const Vec3d& up, double amt){
	auto target = (point - pos()).normalize(-1);
	auto rot = Quatd::getBillboardRotation(target, up);
	quat().slerpTo(rot, amt);
}

Mat4d Pose::matrix() const {
	auto m = quat().toMatrix();
	m.col<3>().xyz() = vec();
	return m;
}

Mat4d Pose::directionMatrix() const {
	auto m = matrix();
	m.col<2>().xyz().negate(); // flip forward vector
	return m;
}

Pose Pose::lerp(const Pose& target, double amt) const {
	Pose r(*this);
	r.pos().lerp(target.pos(), amt);
	r.quat().slerpTo(target.quat(), amt);
	return r;
}

void Pose::toAED(const Vec3d& to, double& az, double& el, double& dist) const {

	auto rel = to - vec();
	dist = rel.mag();

	if(dist > quat().eps()*2.){
		rel.normalize();

		Vec3d ux, uy, uz;
		quat().toCoordinateFrame(ux, uy, uz);

		// dot product of A & B vectors is the similarity or cosine:
		auto xness = rel.dot(ux);
		auto yness = rel.dot(uy);
		auto zness = rel.dot(uz);

		az = -atan2(xness, zness);
		el = asin(yness);
	} else {
		// near origin; might as well assume 0 to avoid denormals
		// do not set az/el; they may already have more meaningful values
		dist = 0.0;
	}
}

void Pose::print() const {
	printf("Vec3d(%f, %f, %f);\nQuatd(%f, %f, %f, %f);\n",
		mVec[0], mVec[1], mVec[2], mQuat[0], mQuat[1], mQuat[2], mQuat[3]);
}



SmoothPose::SmoothPose(const Pose& init, double psmooth, double qsmooth)
:	Pose(init), mTarget(init), mPF(psmooth), mQF(qsmooth)
{}



Nav::Nav(const Vec3d &position, double smooth)
:	Pose(position), mSmooth(smooth), mVelScale(1), mPullBack0(0), mPullBack1(0)
{
}

Nav::Nav(const Nav& nav)
:	Pose(nav.pos(), nav.quat() ),
	mMove0(nav.mMove0), mMove1(nav.mMove1),
	mSpin0(nav.mSpin0), mSpin1(nav.mSpin1),
	mTurn(nav.mTurn), mNudge(nav.mNudge),
	mSmooth(nav.smooth()), mVelScale(nav.mVelScale),
	mPullBack0(nav.mPullBack0), mPullBack1(nav.mPullBack1)
{
}

void Nav::nudgeToward(const Vec3d& p, double amt){
	Vec3d rotEuler;
	Vec3d target(p - pos());
	target.normalize();	// unit vector of direction to move (in world frame)
	// rotate target into local frame:
	target = quat().rotate(target);
	// push ourselves in that particular direction:
	nudge(target * amt);
}

Nav& Nav::halt(){
	mMove0 = 0;
	mMove1 = 0;
	mSpin0 = 0;
	mSpin1 = 0;
	mTurn = 0;
	mNudge = 0;
	return *this;
}

Nav& Nav::home(){
	quat().identity();
	view(0, 0, 0);
	turn(0, 0, 0);
	spin(0, 0, 0);
	mVec = 0;
	return *this;
}

Nav& Nav::view(double azimuth, double elevation, double bank) {
	return view(Quatd().fromEuler(azimuth, elevation, bank));
}

Nav& Nav::view(const Quatd& v) {
	quat(v);
	return *this;
}

Nav& Nav::operator=(const Pose& v){
	Pose::operator=(v);
	return *this;
}

Nav& Nav::operator=(const Nav& v){
	if(&v != this){
		Pose::operator=(v);
		mMove0 = v.mMove0;
		mMove1 = v.mMove1;
		mSpin0 = v.mSpin0;
		mSpin1 = v.mSpin1;
		mTurn = v.mTurn;
		mSmooth = v.mSmooth;
		mVelScale = v.mVelScale;
		mPullBack0 = v.mPullBack0;
		mPullBack1 = v.mPullBack1;
		mTransformed = v.mTransformed;
	}
	return *this;
}

void Nav::step(double dt){
	mVelScale = dt;

	double amt = 1.-smooth();	// TODO: adjust for dt

	// Low-pass filter velocities
	mMove1.lerp(mMove0*dt + mNudge, amt);
	mSpin1.lerp(mSpin0*dt + mTurn , amt);

	// Turn and nudge are a one-shot increments, so clear each step
	mTurn = 0;
	mNudge = 0;

	// Update orientation from smoothed orientation differential
	// Note that vel() returns a smoothed Pose diff from mMove1 and mSpin1.
	mQuat *= vel().quat();

	mQuat.normalize(); // do every n steps for efficiency?
	Vec3d Ur, Uu, Uf;
	directionVectors(Ur, Uu, Uf);

	// Move according to smoothed position differential (mMove1)
	for(int i=0; i<pos().size(); ++i){
		pos()[i] += mMove1.dot(Vec3d(Ur[i], Uu[i], Uf[i]));
	}

	mPullBack1 = mPullBack1 + (mPullBack0-mPullBack1)*amt;

	mTransformed = *this;
	if(mPullBack1 > 1e-16){
		mTransformed.pos() -= Uf * mPullBack1;
	}
}

} // al::
