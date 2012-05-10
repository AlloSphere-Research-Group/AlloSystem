#include <math.h>
#include "allocore/graphics/al_Lens.hpp"

namespace al{

Lens :: Lens(
	double fovy_, 
	double nearClip, 
	double farClip, 
	double focalLength, 
	double eyeSep
)
:	mNear(nearClip),
	mFar(farClip),
	mFocalLength(focalLength),
	mEyeSep(eyeSep)
{
	fovy(fovy_);
}

Lens& Lens::fovy(double v){
	static const double cDeg2Rad = M_PI / 180.;
	mFovy = v;
	mTanFOV = tan(fovy() * cDeg2Rad*0.5);
	return *this;
}

Lens& Lens::fovx(double v, double aspect) {
	fovy(Lens::getFovyForFovX(v, aspect));
	return *this;
}


// @param[in] isStereo		Whether scene is in stereo (widens near/far planes to fit both eyes)
void Lens::frustum(Frustumd& f, const Pose& p, double aspect) const {//, bool isStereo) const {

	Vec3d ur, uu, uf;
	p.directionVectors(ur, uu, uf);
	const Vec3d& pos = p.pos();

	double nh = heightAtDepth(near());
	double fh = heightAtDepth(far());

	double nw = nh * aspect;
	double fw = fh * aspect;
	
//	// This effectively creates a union between the near/far planes of the 
//	// left and right eyes. The offsets are computed by using the law
//	// of similar triangles.
//	if(isStereo){
//		nw += fabs(0.5*eyeSep()*(focalLength()-near())/focalLength());
//		fw += fabs(0.5*eyeSep()*(focalLength()- far())/focalLength());
//	}

	Vec3d nc = pos + uf * near();	// center point of near plane
	Vec3d fc = pos + uf * far();	// center point of far plane

	f.ntl = nc + uu * nh - ur * nw;
	f.ntr = nc + uu * nh + ur * nw;
	f.nbl = nc - uu * nh - ur * nw;
	f.nbr = nc - uu * nh + ur * nw;

	f.ftl = fc + uu * fh - ur * fw;
	f.ftr = fc + uu * fh + ur * fw;
	f.fbl = fc - uu * fh - ur * fw;
	f.fbr = fc - uu * fh + ur * fw;

	f.computePlanes();
}

//double Lens::height(double distance) {
//	return 2*distance * tan(mFovy*M_DEG2RAD*0.5);
//}

//Lens :: Lens(
//	double fovy, 
//	double nearClip, 
//	double farClip, 
//	double focalLength, 
//	double eyeSep,
//	double aspectRatio
//)
//:	Nav(Vec3d(0, 0, -4)),
//	mFovy(fovy),
//	mNear(nearClip),
//	mFar(farClip),
//	mFocalLength(focalLength),
//	mEyeSep(eyeSep),
//	mAspectRatio(aspectRatio),
//	mAutoEyeSep(true),
//	mZoom(0)
//{	smooth(0.8); }


//Matrix4d Lens::modelViewMatrix(Eye e) {
//	switch(e) {
//		case RIGHT: 
//			return Matrix4d::lookAtOffAxis(ur(), uu(), uf(), pos(), mEyeSep);
//		case LEFT: 
//			return Matrix4d::lookAtOffAxis(ur(), uu(), uf(), pos(), -mEyeSep);
//		case MONO:
//		default: 
//			return Matrix4d::lookAt(ur(), uu(), uf(), pos());
//	}
//}

//Matrix4d Lens::projectionMatrix(Eye e) {
//	if(mAutoEyeSep) {
//		mEyeSep = mFocalLength/30.;
//	}
//	switch(e) {
//		case RIGHT: 
//			return Matrix4d::perspectiveOffAxis(mFovy, mAspectRatio, mNear, mFar, mEyeSep, mFocalLength);
//		case LEFT: 
//			return Matrix4d::perspectiveOffAxis(mFovy, mAspectRatio, mNear, mFar, -mEyeSep, mFocalLength);
//		case MONO:
//		default:
//			return Matrix4d::perspective(
//				mFovy, mAspectRatio, mNear, mFar
//			);
//	}
//}


//Frustumd Lens::frustum(Eye eye) {
//	Frustumd f;
//
////	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
////	double fovy		 = mCamera->fovy();
////	double tanFOV = tan(fovy * tanCoef);
////	double aspect	 = mWidth/mHeight;
////	double near		 = mCamera->near();
////	double far		 = mCamera->far();
////	double focal	 = mCamera->focalLength();
////	double IOD		 = mEyeSep * focal/30.0;		// half of inter-ocular distance
////	
////	const Vec3d& pos = mCamera->vec();
////	const Vec3d& ux  = mCamera->ux();
////	const Vec3d& uy  = mCamera->uy();
////	const Vec3d& uz  = mCamera->uz();
////	
////	const Vec3d eye  = pos + (ux * IOD * 0.5);
////	const Vec3d at   = eye + (uz * focal);
////				
////	// Also, update the frustum:
////	const Vec3d& nc = eye - uz * mCamera->near();	// why negative?
////	const Vec3d& fc = eye - uz * mCamera->far();
////
////	
////	f.nh = near * tanFOV;			
////	f.nw = f.nh * mAspect;			
////	f.fh = far  * tanFOV;	
////	f.fw = f.fh * mAspect;
////	
////	f.ntl = nc + uy * f.nh - ux * f.nw;
////	f.ntr = nc + uy * f.nh + ux * f.nw;
////	f.nbl = nc - uy * f.nh - ux * f.nw;
////	f.nbr = nc - uy * f.nh + ux * f.nw;
////
////	f.ftl = fc + uy * f.fh - ux * f.fw;
////	f.ftr = fc + uy * f.fh + ux * f.fw;
////	f.fbl = fc - uy * f.fh - ux * f.fw;
////	f.fbr = fc - uy * f.fh + ux * f.fw;
////	
////	f.pl[Frustumd::TOP].set3Points(	f.ntr,	f.ntl,	f.ftl);
////	f.pl[Frustumd::BOTTOM].set3Points(f.nbl,	f.nbr,	f.fbr);
////	f.pl[Frustumd::LEFT].set3Points(	f.ntl,	f.nbl,	f.fbl);
////	f.pl[Frustumd::RIGHT].set3Points(	f.nbr,	f.ntr,	f.fbr);
////	f.pl[Frustumd::NEARP].set3Points(	f.ntl,	f.ntr,	f.nbr);
////	f.pl[Frustumd::FARP].set3Points(	f.ftr,	f.ftl,	f.fbl);
//	
//	return f;
//}

} // al::
