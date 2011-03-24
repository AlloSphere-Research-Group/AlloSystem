#ifndef INCLUDE_AL_NI_HPP
#define INCLUDE_AL_NI_HPP

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/



#include "allocore/system/al_Time.hpp"
#include "allocore/system/al_Thread.hpp"
#include "allocore/types/al_Array.hpp"

#include <list>

namespace al{

class Ni {
public:
	static Ni& get();

	void listDevices();

protected:
	Ni();	// singleton pattern.
};


class Kinect {
public:

	class Callback {
	public:
		virtual ~Callback() {}
		virtual void onKinectData(Kinect& k) = 0;
	};

	Kinect(unsigned deviceID);
	~Kinect();

	bool start();
	bool stop();

	// 640x480, float32, 1 component
	Array& depthArray() { return mDepthArray; }
	Array& rawDepthArray() { return mRawDepthArray; }
	Array& realWorldArray() { return mRealWorldArray; }
	double fps() const { return mFPS; }

	void add(Callback * cb) {
		mCallbacks.push_back(cb);
	}

	static void * getContext();

	// these values are populated when the camera is initialized:
	// focal length in mm (Zero Plane Distance)
	uint64_t zpd() const { return mZPD; }
	// pixel size in mm (at Zero Plane)
	double zpps() const { return mZPPS; }

	// from raw depth in mm:
	Vec3d toRealWorld(int u, int v, int depth) {
		const double meters = depth * 0.001;
		const double metersPerPixel = meters * 2. * mZPPS / mZPD;
		return Vec3d(
			(u-320) * metersPerPixel,
			(v-240) * metersPerPixel,
			meters
		);
	}

protected:
	class Impl;
	Impl * mImpl;
	Array mDepthArray;
	Array mRawDepthArray;
	Array mRealWorldArray;
	al_sec mTime;
	double mFPS;
	bool mDepthNormalize;
	bool mActive;
	Thread mThread;
	std::list<Callback *> mCallbacks;
	unsigned mDeviceID;

	uint64_t mZPD;
	double mZPPS;

	static void * threadFunction(void * userData);
	bool tick();


};

} // al::

#endif /* include guard */
