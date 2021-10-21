#ifndef INCLUDE_AL_WATCHER_HPP
#define INCLUDE_AL_WATCHER_HPP

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
	Utility for registering & recieving notifications

	File author(s):
	Graham Wakefield, 2011, grrrwaaa@gmail.com
*/

#include <string>

namespace al {

///! base class for all notifiable objects:
///	MyWatcher w;
/// w.watch("foo");
/// Watcher::notify("foo", "bar");
///
/// @ingroup allocore
class Watcher {
public:
	/// destructor automatically un-registers:
	virtual ~Watcher() { unwatch(); }

	/// get notifications from a named resource
	void watch(std::string resourcename);

	/// stop notifications from a named resource
	void unwatch(std::string resourcename);
	/// stop all notifications
	void unwatch();

	/// the notification handler:
	virtual void onEvent(std::string resourcename, std::string eventname) {}

	/// trigger a notification for a named resource
	static void notify(std::string resourcename, std::string eventname);
};

} //al::

#endif
