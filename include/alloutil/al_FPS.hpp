#ifndef INCLUDE_AL_UTIL_FPS_HPP
#define INCLUDE_AL_UTIL_FPS_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
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
