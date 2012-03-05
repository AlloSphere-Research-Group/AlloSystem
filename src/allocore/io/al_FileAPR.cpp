#include <cstring>

#include "allocore/io/al_File.hpp"

#include "../private/al_ImplAPR.h"
#ifdef AL_LINUX
	#include "apr-1.0/apr_file_info.h"
#else
	#include "apr-1/apr_file_info.h"
#endif

namespace al{

//void path2dir(char* dst, const char* src) {
//    char* s;
//	snprintf(dst, AL_PATH_MAX, "%s", src);
//    s = strrchr(dst, '/');
//    if (s)
//        s[1] = '\0';
//    else
//        dst[0] = '\0';
//}

struct File::Impl : public ImplAPR {
public:
	mutable apr_finfo_t finfo;
	
	Impl() : ImplAPR() {}
	virtual ~Impl() {}

	bool getInfo(const char * path, apr_int32_t wanted) const {
		return (APR_SUCCESS == check_apr(apr_stat(&finfo, path, wanted, mPool)));
	}
};

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
			printf("couldn't open directory %s\n", dirname.c_str());
		}
		return found;
	}
};


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

FilePath::FilePath(std::string fullpath) {
	size_t found = fullpath.rfind(AL_FILE_DELIMITER);
	if (found!=std::string::npos) {
		mPath = fullpath.substr(0, found+1);
		mFile = fullpath.substr(found+1);
	} else {
		mPath = "/";
		mFile = fullpath;
	}

}

File::File(const std::string& path, const std::string& mode, bool open_)
:	mImpl(new Impl()), 
	mPath(path), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::File(const FilePath& path, const std::string& mode, bool open_)
:	mImpl(new Impl()), 
	mPath(path.filepath()), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::~File(){ close(); freeContent(); delete mImpl; }

void File::allocContent(int n){
	if(mContent) freeContent();
	mContent = new char[n+1];
	mContent[n] = '\0';
}


void File::freeContent(){ delete[] mContent; }

void File::getSize(){
	int r=0;
	if(opened()){
		fseek(mFP, 0, SEEK_END);
		r = ftell(mFP);
		rewind(mFP);
//		printf("File::getSize %d (%s)\n", r, mode().c_str());
	}
	mSizeBytes = r;
}

bool File::open(){
	if(0 == mFP){
		mFP = fopen(path().c_str(), mode().c_str());
		if(mFP){
			getSize();
			return true;
		}
	}
	return false;
}

void File::close(){ if(opened()){ fclose(mFP); } mFP=0; mSizeBytes=0; }

const char * File::readAll(){
	if(opened() && mMode[0]=='r'){
		int n = size();
		allocContent(n);
		int numRead = fread(mContent, sizeof(char), n, mFP);
		if(numRead < n){}
	}
	return mContent;
}

int File::write(const std::string& path, const void * v, int size, int items){
	File f(path, "w");
	int r = 0;
	if(f.open()){
		r = f.write(v, size, items);
		f.close();
	}
	return r;
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


} // al::

