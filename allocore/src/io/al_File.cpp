#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include "allocore/io/al_File.hpp"

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

void File::dtor(){
	close();
	freeContent();
}

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

void File::close(){
	if(opened()) fclose(mFP);
	mFP=0;
	mSizeBytes=0;
}

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

int File::write(const std::string& path, const std::string& data){
	return File::write(path, &data[0], data.size());
}



std::string File::conformDirectory(const std::string& src){
	if(src[0]){
		if(AL_FILE_DELIMITER != src[src.size()-1]){
			return src + AL_FILE_DELIMITER;
		}
		return src;
	}
	return "." AL_FILE_DELIMITER_STR;
}

std::string File::conformPathToOS(const std::string& src){
	std::string res(src);

	// Ensure delimiters are correct
	for(unsigned i=0; i<src.size(); ++i){
		char c = res[i];
		if('\\'==c || '/'==c){
			res[i] = AL_FILE_DELIMITER;
		}
	}

	// Ensure that directories end with a delimiter
	if(isDirectory(res)){
		return conformDirectory(res);
	}
	return res;
}

std::string File::absolutePath(const std::string& src) {
#ifdef AL_WINDOWS
	char temp[_MAX_PATH];
	char * result = _fullpath(temp, src.c_str(), sizeof(temp));
	return result ? result : "";
#else
	char temp[PATH_MAX];
	char * result = realpath(src.c_str(), temp);
	return result ? result : "";
#endif
}

std::string File::baseName(const std::string& src){
	size_t pos = src.find_last_of('.');
	return src.substr(0, pos);
}

std::string File::directory(const std::string& src){
	size_t pos = src.find_last_of(AL_FILE_DELIMITER);
	if(std::string::npos != pos){
		return src.substr(0, pos+1);
	}
	return "." AL_FILE_DELIMITER_STR;
}

std::string File::extension(const std::string& src){
	size_t pos = src.find_last_of('.');
	if(src.npos != pos){
		return src.substr(pos);
	}
	return "";
}

bool File::exists(const std::string& path){
	struct stat s;
	return ::stat(path.c_str(), &s) == 0;
}

bool File::isDirectory(const std::string& src){
	struct stat s;
	if(0 == ::stat(src.c_str(), &s)){	// exists?
		if(s.st_mode & S_IFDIR){		// is dir?
			return true;
		}
	}
	// if(s.st_mode & S_IFREG) // is file?
	return false;
}

bool File::searchBack(std::string& prefixPath, const std::string& matchPath, int maxDepth){
	if(prefixPath[0]){
		prefixPath = conformDirectory(prefixPath);
	}
	int i=0;
	for(; i<maxDepth; ++i){
		if(File::exists(prefixPath + matchPath)) break;
		prefixPath += ".." AL_FILE_DELIMITER_STR;
	}
	return i<maxDepth;
}

bool File::searchBack(std::string& path, int maxDepth){
	std::string prefix = "";
	bool r = searchBack(prefix, path);
	if(r) path = prefix + path;
	return r;
}




FilePath::FilePath(const std::string& fullpath) {
	size_t found = fullpath.rfind(AL_FILE_DELIMITER);
	if (found!=std::string::npos) {
		mPath = fullpath.substr(0, found+1);
		mFile = fullpath.substr(found+1);
	} else {
		mPath = AL_FILE_DELIMITER_STR;
		mFile = fullpath;
	}

}




SearchPaths::SearchPaths(const std::string& file) {
	FilePath fp(file);
	addAppPaths(fp.path());
}

SearchPaths::SearchPaths(int argc, char * const argv[], bool recursive){
	addAppPaths(argc,argv,recursive);
}

SearchPaths::SearchPaths(const SearchPaths& cpy) 
:	mSearchPaths(cpy.mSearchPaths),
	mAppPath(cpy.mAppPath)
{}

void SearchPaths::addAppPaths(std::string path, bool recursive) {
	std::string filepath = File::directory(path);
	mAppPath = filepath;
	addSearchPath(filepath, recursive);
}

void SearchPaths::addAppPaths(int argc, const char ** argv, bool recursive) {
	addAppPaths(recursive);
	if (argc > 0) {
		addAppPaths(File::directory(argv[0]), recursive);
	} 
}

void SearchPaths::addAppPaths(int argc, char * const argv[], bool recursive) {
	addAppPaths(recursive);
	if (argc > 0) {
		addAppPaths(File::directory(argv[0]), recursive);
	} 
}

void SearchPaths::addAppPaths(bool recursive) {
	char cwd[4096];
	if(getcwd(cwd, sizeof(cwd))){
		mAppPath = std::string(cwd) + "/";
		addSearchPath(mAppPath, recursive);
	}
}

void SearchPaths::addSearchPath(const std::string& src, bool recursive) {
	std::string path=File::conformDirectory(src);

	// check for duplicates
	std::list<searchpath>::iterator iter = mSearchPaths.begin();
	while (iter != mSearchPaths.end()) {
		//printf("path %s\n", iter->first.c_str());
		if (path == iter->first) {
			return;
		}
		iter++;
	}
//	printf("adding path %s\n", path.data());
	mSearchPaths.push_front(searchpath(path, recursive));
}

void SearchPaths::print() const {
	printf("SearchPath %p appPath: %s\n", this, appPath().c_str());
	std::list<searchpath>::const_iterator it = mSearchPaths.begin();
	while (it != mSearchPaths.end()) {
		const SearchPaths::searchpath& sp = (*it++);
		printf("SearchPath %p path: %s (recursive: %d)\n", this, sp.first.c_str(), sp.second);
	}
}


} // al::

