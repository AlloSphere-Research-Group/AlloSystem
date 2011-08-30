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
		printf("error shot %s not found\n", shotname.c_str());
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
