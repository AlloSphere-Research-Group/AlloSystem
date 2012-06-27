#include "allocore/system/al_Watcher.hpp"

#include <map>
#include <set>

using namespace al;

typedef std::set<Watcher *> Watchers;
typedef std::map<std::string, Watchers> WatchersMap;	

/// singleton notification center:
static WatchersMap gWatchers;

void Watcher::watch(std::string name) {
	Watchers& w = gWatchers[name];
	w.insert(this);
}

void Watcher::unwatch(std::string name) {
	Watchers& w = gWatchers[name];
	w.erase(this);
}

void Watcher::unwatch() {
	for (WatchersMap::iterator it=gWatchers.begin(); it!=gWatchers.end(); it++) {
		unwatch(it->first);
	}
}

void Watcher::notify(std::string name, std::string event) {
	Watchers& w = gWatchers[name];
	for (Watchers::iterator it=w.begin(); it!=w.end(); it++) {
		Watcher * rw = *it;
		rw->onEvent(name, event);
	}
}


