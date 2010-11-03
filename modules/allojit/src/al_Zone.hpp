#ifndef INCLUDE_AL_ZONE_HPP
#define INCLUDE_AL_ZONE_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

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

#include <list>

namespace al {
	


///*
//	A zone forms a tree-like hierarchy
//		When a parent zone is destroyed, all child zones are destroyed (in reverse order of creation)
//*/
//class Zone {
//public:
//
//	Zone(Zone * parent) : mParentZone(parent) {}
//	
//	~Zone() {
//		while (!mChildZones.empty()) {
//			delete *mChildZones.rbegin();
//		}
//		if (mParentZone) mParentZone->remove(this);
//	}
//	
//	Zone * create() {
//		Zone * z = new Zone(this);
//		add(z);
//		return z;
//	}
//
//	void add(Zone * z) {
//		mChildZones.push_back(z);
//	}
//	
//	void remove(Zone * z) {
//		mChildZones.remove(z);
//	}
//
//	Zone * mParentZone;
//	std::list<Zone *> mChildZones;
//
//};
	
	
} // al::

#endif /* include guard */
