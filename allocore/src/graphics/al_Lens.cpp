#include <cmath>
#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Frustum.hpp"
#include "allocore/spatial/al_Pose.hpp"
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

double Lens::getFovyForHeight(double height, double depth) {
	return 2.*M_RAD2DEG*atan(height/depth);
}

double Lens::getFovyForFovX(double fovx, double aspect) {
	double farW = tan(0.5*fovx*M_DEG2RAD);
	return 2.*M_RAD2DEG*atan(farW/aspect);
}

} // al::
