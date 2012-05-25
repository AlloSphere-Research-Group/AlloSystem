#ifndef INCLUDE_AL_UTIL_FPS_HPP
#define INCLUDE_AL_UTIL_FPS_HPP

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
	Utility for measuing FPS

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/system/al_Time.hpp"

namespace al {
	
class FPS {
public:
	
	FPS() : lastFrameT(al_time()), dt(0.1), frame(0), fps_measured(10) { birth = lastFrameT; }
	
	void onFrame() {
		// timing:
		al_sec f = al_time();
		dt = f-lastFrameT;
		lastFrameT = f;
		fps_measured = fps_measured + 0.1 * ((1./dt) - fps_measured);
		frame++;
	}
	
	// get current time:
	al_sec now() { return al_time() - birth; }
	
	double fps() { return fps_measured; }
	
	al_sec birth, lastFrameT, dt;
	unsigned frame;
	double fps_measured;
};

} //al::

#endif
