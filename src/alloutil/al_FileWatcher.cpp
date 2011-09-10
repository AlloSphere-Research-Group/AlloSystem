#include "alloutil/al_FileWatcher.hpp"

#include <vector>
#include <map>
#include <limits>

using namespace al;

typedef std::vector<FileWatcher *> WatcherList;

struct WatchedFile {
	WatchedFile() : mModified(-std::numeric_limits<double>::max()) {}
	WatchedFile(const WatchedFile& cpy) : mModified(cpy.mModified) {}
	
	void add(FileWatcher * watcher) {
		mWatchers.push_back(watcher);
	}
	
	void remove(FileWatcher * watcher) {
		WatcherList::iterator iter = mWatchers.begin();
		while (iter != mWatchers.end()) {
			if (*iter == watcher) {
				iter = mWatchers.erase(iter);
			} else {
				iter++;
			}
		}
	}
	
	void test(FileWatcher * w) {	
		if (File::exists(mPath)) {
			File f(mPath, "r", false);
			al_sec mod = f.modified();
			if(mod > mModified) {
				f.open();
				WatcherList::iterator iter = mWatchers.begin();
				while (iter != mWatchers.end()) {
					FileWatcher * fw = *iter++;
					if (w == fw) 
						w->onFileWatch(f);
				}
				f.close();
				mModified = f.modified();
			}
		}
	}
	
	void test() {	
		if (File::exists(mPath)) {
			File f(mPath, "r", false);
			al_sec mod = f.modified();
			if(mod > mModified) {
				notify(f);
			}
		}
	}
	
	void notify(File& f) {		
		f.open();
		WatcherList::iterator iter = mWatchers.begin();
		while (iter != mWatchers.end()) {
			(*iter++)->onFileWatch(f);
		}
		f.close();
		mModified = f.modified();
	}
	
	std::string mPath;
	al_sec mModified;
	WatcherList mWatchers;
};

typedef std::map<std::string, WatchedFile > WatcherMap;

WatcherMap gWatchedFiles;
al_sec gPollPeriod;

void FileWatcher::poll() {
	// check each file:
	WatcherMap::iterator it = gWatchedFiles.begin();
	while (it != gWatchedFiles.end()) {
		it->second.test(this);
		it++;
	}
}

void FileWatcher::pollAll() {
	// check each file:
	WatcherMap::iterator it = gWatchedFiles.begin();
	while (it != gWatchedFiles.end()) {
		it->second.test();
		it++;
	}
}

static void autopoll(al_sec t) {
	FileWatcher::pollAll();
	if (gPollPeriod > 0.) MainLoop::queue().send(t+gPollPeriod, autopoll);
}


void FileWatcher::autoPoll(al_sec t) {
	if (t > 0.) {
		if (gPollPeriod == 0.) {
			gPollPeriod = t;
			// start the task:
			autopoll(MainLoop::now());
		} else {
			gPollPeriod = t;
		}
	} else {
		// turn it off:
		gPollPeriod = 0.;
	}
}



FileWatcher::~FileWatcher(){ 
	// check each file:
	WatcherMap::iterator it = gWatchedFiles.begin();
	while (it != gWatchedFiles.end()) {
		it->second.remove(this);
		it++;
	}
}

void FileWatcher::watch(std::string filepath, bool immediate) {
	// find or create:
	WatchedFile& wf = gWatchedFiles[filepath];
	wf.mPath = filepath;
	wf.add(this);
	if (immediate) wf.test(); 
}



///// Singleton object to manage filewatching notifications
///// Starts polling in the MainLoop scheduler as soon as FileWatcherManager::get() is called.
//class FileWatcherManager {
//public:
//	struct WatchedFile {
//		WatchedFile(std::string path, FileWatcher * handler)
//		: mPath(path), mHandler(handler), mModified(0) {}
//		WatchedFile(const WatchedFile& cpy)
//		: mPath(cpy.mPath), mHandler(cpy.mHandler), mModified(cpy.mModified) {}
//		
//		void operator()() {
//			File f(mPath, "r", false);
//			if (File::exists(mPath)) {
//				al_sec mod = f.modified();
//				if(mod > mModified) {
//					f.open();
//					mHandler->onFileWatch(f);
//					f.close();
//					mModified = f.modified();
//				}
//			}
//		}
//		
//		std::string mPath;
//		al_sec mModified;
//		FileWatcher * mHandler;
//	};
//	typedef std::vector<WatchedFile> WatcherMap;
//
//	/// access the singleton:
//	static FileWatcherManager& get() {
//		static FileWatcherManager singleton;
//		return singleton;
//	}
//	
//	FileWatcherManager() 
//	:	mPeriod(1.),
//		mActive(false)
//	{
//		// start polling:
//		poll(MainLoop::now());
//	}
//	
//	~FileWatcherManager() {}
//	
//	/// add a file to watch:
//	/// this may overwrite an existing handler.
//	/// param[in] immediate: triggers the handler immediately if the file exists
//	FileWatcherManager& watch(std::string path, FileWatcher * handler, bool immediate) {
//		if (File::exists(path)) {
//			// note this may overwrite an existing handler.
//			WatchedFile wf(path, handler);
//			if (immediate) {
//				wf();
//			} else {
//				wf.mModified = File::modified(path);
//			}
//			mHandlers.push_back(wf);
//		} else {
//			printf("warning: attempt to watch non-existent file %s\n", path.c_str());
//		}
//		return *this;
//	}
//	
//	/// stop watching a file:
//	FileWatcherManager& unwatch(FileWatcher * handler) {
//		// one watcher may handle several files:
//		WatcherMap::iterator iter = mHandlers.begin();
//		while (iter != mHandlers.end()) {
//			WatchedFile& wf = *iter++;
//			if (handler == wf.mHandler) mHandlers.erase(iter);
//		}
//		return *this;
//	}
//	
//	/// set/get polling period:
//	FileWatcherManager& period(al_sec p) {
//		p = fabs(p);
//		p = p < 0.01 ? 0.01 : p;
//		mPeriod = p;
//		return *this;
//	}
//	al_sec period() { return mPeriod; }
//	
//	// scheduler task function
//	void poll(al_sec t) {
//		WatcherMap::iterator iter = mHandlers.begin();
//		while (iter != mHandlers.end()) {
//			WatchedFile& wf = *iter++;
//			wf();
//		}
//		MainLoop::queue().send(t+mPeriod, this, &FileWatcherManager::poll);
//	}
//	
//	WatcherMap mHandlers;
//	al_sec mPeriod;
//	bool mActive;
//};
