#ifndef INCLUDE_AL_NI_HPP
#define INCLUDE_AL_NI_HPP

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
	Binding to OpenNI

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Ritesh Lala, 2010, riteshlala.ed@gmail.com
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
	
	unsigned numDevices() { return mNumDevices; }

protected:
	Ni();	// singleton pattern.
	unsigned mNumDevices;
};


class Kinect {
public:

	// Warning: triggered in the Kinect's own thread
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
//	Array& realWorldArray() { return mRealWorldArray; }
	double fps() const { return mFPS; }

	void add(Callback * cb) {
		mCallbacks.push_back(cb);
	}
	
	unsigned id() { return mDeviceID; }

	static void * getContext();

	// these values are populated when the camera is initialized:
	// focal length in mm (Zero Plane Distance)
	uint64_t zpd() const { return mZPD; }
	// pixel size in mm (at Zero Plane)
	double zpps() const { return mZPPS; }
	
	uint16_t zres() const { return mZRes; }

	// from pixel location & raw depth in mm
	// returns location in meters:
	Vec3f toRealWorld(int u, int v, int depth) {
		const double meters = depth * 0.001;
		const double metersPerPixel = meters * 2. * mZPPS / mZPD;
		return Vec3f(
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
//	Array mRealWorldArray;
	al_sec mTime;
	double mFPS;
	bool mDepthNormalize;
	bool mActive;
	Thread mThread;
	std::list<Callback *> mCallbacks;
	unsigned mDeviceID;

	uint64_t mZPD;
	double mZPPS;
	uint16_t mZRes;

	static void * threadFunction(void * userData);
	bool tick();


};

} // al::

#endif /* include guard */
