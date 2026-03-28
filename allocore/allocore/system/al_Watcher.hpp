#ifndef INC_AL_WATCHER_HPP
#define INC_AL_WATCHER_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Utility for registering & recieving notifications

	Author(s):
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
