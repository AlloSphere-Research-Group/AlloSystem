#include <cstring>
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"

// TODO:
// At the moment, we only use APR for getting modification times of files
// and for scanning directories. It would be better to have native solutions
// for these two tasks...

#include "../private/al_ImplAPR.h"
#ifdef AL_LINUX
	#include "apr-1.0/apr_file_info.h"
#else
	#include "apr-1/apr_file_info.h"
#endif

namespace al{

FileInfo::Type fromAPRFileType(int aprEnum){
	switch(aprEnum){
	case APR_NOFILE: return FileInfo::NOFILE;
	case APR_REG: return FileInfo::REG;
	case APR_DIR: return FileInfo::DIR;
	case APR_CHR: return FileInfo::CHR;
	case APR_BLK: return FileInfo::BLOCK;
	case APR_PIPE: return FileInfo::PIPE;
	case APR_LNK: return FileInfo::LINK;
	case APR_SOCK: return FileInfo::SOCKET;
	default: return FileInfo::UNKNOWN;
	}
}


struct File::Impl : public ImplAPR {
public:
	mutable apr_finfo_t finfo;
	
	Impl() : ImplAPR() {}
	virtual ~Impl() {}

	bool getInfo(const char * path, apr_int32_t wanted) const {
		return (APR_SUCCESS == check_apr(apr_stat(&finfo, path, wanted, mPool)));
	}
};


File::File(const std::string& path, const std::string& mode, bool open_)
:	mImpl(new Impl()), 
	mPath(path), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::File(const FilePath& path, const std::string& mode, bool open_)
:	mImpl(new Impl()), 
	mPath(path.filepath()), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::~File(){
	dtor();
	delete mImpl;
}

al_sec File :: modified() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_MTIME)) {
		return 1.0e-6 * al_sec(mImpl->finfo.mtime);
	}
	return 0;
}

al_sec File :: accessed() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_ATIME)) {
		return 1.0e-6 * al_sec(mImpl->finfo.atime);
	}
	return 0;
}

al_sec File :: created() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_CTIME)) {
		return 1.0e-6 * al_sec(mImpl->finfo.ctime);
	}
	return 0;
}

size_t File :: sizeFile() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_SIZE)) {
		return mImpl->finfo.size;
	}
	return 0;
}

size_t File :: storage() const {
	if (mImpl->getInfo(path().c_str(), APR_FINFO_CSIZE)) {
		return mImpl->finfo.csize;
	}
	return 0;
}



struct Dir::Impl : public ImplAPR{
	apr_finfo_t finfo;
	apr_dir_t * dir;
	
	Impl(): dir(NULL){}

	Impl(const std::string& dirName): dir(NULL){
		open(dirName);
	}
	
	virtual ~Impl(){
		close();
	}
	
	bool open(const std::string& dirName){
		return APR_SUCCESS == 
			check_apr(apr_dir_open(&dir, dirName.c_str(), mPool));
	}
	
	bool close(){
		//if(dir != NULL){
			return APR_SUCCESS == check_apr(apr_dir_close(dir));
		//}
		//return false;
	}

	bool read(FileInfo& entryInfo){
		// Calling check_apr frequently prints out "No such file or directory"
		// messages, so disabling for now...
		if(APR_SUCCESS == 
			/*check_apr*/(apr_dir_read(&finfo, APR_FINFO_TYPE | APR_FINFO_NAME, dir)))
		{
			entryInfo.type(fromAPRFileType(finfo.filetype));
			entryInfo.name(finfo.name);
			return true;
		}
		return false;
	}

	bool rewind(){
		return APR_SUCCESS == check_apr(apr_dir_rewind(dir));
	}
};


Dir::Dir()
:	mImpl(new Impl)
{}

Dir::Dir(const std::string& dirToOpen)
:	mImpl(new Impl(dirToOpen))
{}

Dir::~Dir(){
	delete mImpl;
}

bool Dir::open(const std::string& dirPath){ return mImpl->open(dirPath); }
bool Dir::close(){ return mImpl->close(); }
bool Dir::rewind(){ return mImpl->rewind(); }
bool Dir::read(){ return mImpl->read(mEntry); }

/*
struct TestDir{
	TestDir(){
		Dir dir;
		if(dir.open("/")){
			for(int i=0; i<1; ++i){
				dir.rewind();
				while(dir.read()){
					printf("%c %s\n", dir.entry().type() == FileInfo::DIR ? 'd':' ', dir.entry().name().c_str());
				}
			}
		}
	}
} testDir;
*/



// Note: this class is used only by SearchPaths::find.
class Path : public ImplAPR {
public:
	apr_dir_t * dir;
	apr_finfo_t dirent;
	std::string dirname;
	
	Path(const std::string& dirname) : ImplAPR(), dir(NULL), dirname(dirname) {
		if (APR_SUCCESS != check_apr(apr_dir_open(&dir, dirname.c_str(), mPool))) {
			//printf("dir %p\n", dir);
			dir=NULL;
		}
	}
	
	virtual ~Path() {
		if (dir != NULL) check_apr(apr_dir_close(dir));
	}
	
	bool find(const std::string& name, FilePath& result, bool recursive=true) {
		bool found = false;
		if (dir && APR_SUCCESS == check_apr(apr_dir_open(&dir, dirname.c_str(), mPool))) {
			// iterate over directory:
			while ((!found) && APR_SUCCESS == (apr_dir_read(&dirent, APR_FINFO_TYPE|APR_FINFO_NAME, dir))) {
				//printf("test %s %s\n", dirname.c_str(), dirent.name);
				if (dirent.filetype == APR_REG && dirent.name && std::string(dirent.name) == name) {
					result.file(dirent.name);
					result.path(dirname);
					found = true;
					break;
				} else if (recursive && dirent.filetype == APR_DIR && dirent.name && dirent.name[0] != '.') {
					Path path(dirname + dirent.name + AL_FILE_DELIMITER);
					found = path.find(name, result, true);
				}
			}
		} else {
			AL_WARN("couldn't open directory %s", dirname.c_str());
		}
		return found;
	}
};


// This is the one sour egg that needs APR impl for directory scanning...
FilePath SearchPaths::find(const std::string& name) {
	FilePath result;
	bool found = false;
	std::list<SearchPaths::searchpath>::iterator iter = mSearchPaths.begin();
	while ((!found) && iter != mSearchPaths.end()) {
		Path path(iter->first.c_str());
		found = path.find(name, result, iter->second);
		iter++;
	}
	return result;
}

} // al::

