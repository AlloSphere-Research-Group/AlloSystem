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
	Utils for building an animation as a series of shots and events
	
	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Pablo Colapinto, 2010, wolftype@gmail.com
*/

/*
 *  Animation.h
 *  nanomed
 *
 *  Created by x on 6/5/11.
 *  Copyright 2011 x. All rights reserved.
 *
 
 utils for building an animation as a series of shots and events
 */

#ifndef INC_SHOTLIST_H
#define INC_SHOTLIST_H

#include "allocore/al_Allocore.hpp"

namespace al {

/*
	A Shot has a duration, start and end events
*/
class Shot {
public:
	Shot(std::string name, al_sec duration) 
	: mName(name), mDuration(duration) 
	{}
	
	std::string name() const { return mName; }
	al_sec duration() const { return mDuration; }
	
	/// execute actions at short start:
	virtual void onStart() {}
	
	/// execute actions at shot end:
	virtual void onEnd() {}
	
	/// execute actions per frame:
	/// dt is seconds since last frame
	/// phase runs from 0..1 over the shot (shot time is phase * duration())
	virtual void onFrame(al_sec dt, double phase) {}

	std::string mName;
	al_sec mDuration;
	
	MsgQueue events;
};

/*
	ShotList connects a series of shots
	
*/
class ShotList {
public:
	typedef std::vector<Shot *> Container;
	
	ShotList() : mNow(0.), mAutoRewind(true) {
		//mShot = mShots.begin();
		mShot = 0;
	}
	
	/// return true if shot list has ended (no valid current shot)
	bool ended() const { return mShot >= mShots.size(); }
	
	/// return ptr to current shot, or NULL if ended:
	Shot * current() const { return ended() ? NULL : mShots[mShot]; }

	/// append a shot to the list:
	void add(Shot * shot) { mShots.push_back(shot); }
	
	// execute the shot list per frame
	void operator()(al_sec dt) {
		Shot * shot = current();
		if (shot == NULL) return;
		
		al_sec dur = shot->duration();
		al_sec t2 = mNow + dt;
		
		// update current shot:
		if (mNow <= 0.) {
			shot->onStart();
		}
		
		double phase = t2/dur;
		
		//execute shot events:
		//shot->events.update(phase);
		
		if (phase < 1.) {
			mNow = t2;
			// render it:
			shot->onFrame(dt, phase);
		} else {
			// fire onFrame for t == duration
			shot->onFrame(dur - mNow, 1.);			
			// move to next shot:
			next();
		}
	}
	
	/// jump to the first shot:
	void rewind() {
		//jump(mShots.begin());
		jump(0);
	}
	
	/// jump to just after the end of the animation:
	void end() {
		//jump(mShots.end());
		jump(mShots.size());
	}
	
	/// skip to the next shot:
	void next() {
		if (!ended()) jump(mShot+1);
		if (ended() && mAutoRewind) rewind();
	}
	
	// jump to a particular shot by name:
	void jump(std::string shotname) {
		for (int i=0; i<mShots.size(); i++) {
			if (mShots[i]->name() == shotname) {
				jump(i);	// jump by iterator
				return;
			}
		}
		AL_WARN("error shot %s not found", shotname.c_str());
	}

	// jump to particular shot by iterator:
	void jump(unsigned idx) {
		// if it is different to current:
		if (idx != mShot) {
			// end current:
			Shot * shot = current();
			if (shot) shot->onEnd();
			// change
			mShot = idx;
		}
		// reset clock:
		mNow = 0;
	}
	
protected:	
	
	al_sec mNow;
	Container mShots;
	unsigned mShot;
	
	bool mAutoRewind;
	
};
	
class BlankShot : public Shot {
public:
	
	BlankShot(al_sec dur=1) : Shot("blank", dur) {}
	virtual ~BlankShot(){}		
	
};
	
class ExitShot : public Shot {
public:
	
	ExitShot() : Shot("exit", 1) {}
	virtual ~ExitShot(){}		
	virtual void onStart(){ exit(0); }
	
};

} //al::

#endif
