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
	Kinect(unsigned deviceID);
	~Kinect();
	
	bool start();
	bool stop();
	
	
	Array& depthArray() { return mDepthArray; }
	double fps() const { return mFPS; }

protected:
	class Impl;
	Impl * mImpl;
	Array mDepthArray;
	al_sec mTime;
	double mFPS;
	bool mDepthNormalize;
	bool mActive;
	Thread mThread;
	
	static void * threadFunction(void * userData);
	bool tick();
	
};

} // al::

#endif /* include guard */
