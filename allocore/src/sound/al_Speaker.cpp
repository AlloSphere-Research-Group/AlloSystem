#include "allocore/sound/al_Speaker.hpp"
#include <cmath> // sin, cos, atan2, sqrt

using namespace al;

#define DEG2RAD 0.017453292519943f
#define RAD2DEG 57.295779513082322f

Speaker::Speaker(int deviceChan, float az, float el, float radius, float gain)
:	deviceChannel(deviceChan), gain(gain), azimuth(az), elevation(el), radius(radius)
{}

Speaker& Speaker::pos(Vec3f p){
	radius = p.mag();
	auto gd = p.xy().mag();
	elevation = atan2(p.z, gd) * RAD2DEG;
	  azimuth = atan2(p.x,p.y) * RAD2DEG;
	return *this;
}

Vec3f Speaker::pos() const {
	float elr = elevation * DEG2RAD;
	float azr = azimuth * DEG2RAD;
	float cosel = cos(elr);
	return Vec3f(
		sin(azr) * cosel,
		cos(azr) * cosel,
		sin(elr)
	) * radius;
	//Ryan: the standard conversions assume +z is up, these are correct for allocore
	/*return Vec3f(
		 sin(azr) * cosel,
		 sin(elr),
		-cos(azr) * cosel
	) * radius;*/
}



int SpeakerLayout::numSpeakers() const {
	return mSpeakers.size();
}

SpeakerLayout& SpeakerLayout::addSpeaker(const Speaker& s){
	mSpeakers.push_back(s);
	return *this;
}


HeadsetSpeakerLayout::HeadsetSpeakerLayout(int deviceChannelStart, float radius, float gain)
:	SpeakerRingLayout<2>(deviceChannelStart, 90, radius, gain)
{}



StereoSpeakerLayout::StereoSpeakerLayout(int deviceChannelStart, float angle, float distance, float gain)
:	mLeft(deviceChannelStart, angle, 0, distance, gain),
	mRight(deviceChannelStart + 1, -angle, 0, distance, gain)
{
	addSpeaker(mLeft);
	addSpeaker(mRight);
}



CubeLayout::CubeLayout(int deviceChannelStart){
	mSpeakers.reserve(8);
	for(int i=0; i<4; ++i) {
		addSpeaker({    i+deviceChannelStart, 45.f + i*90,  0           });
		addSpeaker({4 + i+deviceChannelStart, 45.f + i*90, 60, sqrt(5.f)});
	}
}
