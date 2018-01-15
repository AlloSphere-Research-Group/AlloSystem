#include <cstring>
#include <fstream> // ifstream, ofstream
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>
#include <regex>
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Config.h"
#include <stdlib.h> // realpath (POSIX), _fullpath (Windows)
#ifdef AL_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define VC_EXTRALEAN
	#include <windows.h> // TCHAR, LPCTSTR
	#include <direct.h> // _getcwd
	#define platform_getcwd _getcwd
	#ifndef PATH_MAX
	#define PATH_MAX 260
	#endif
#else
	#include <unistd.h> // getcwd (POSIX)
	#define platform_getcwd getcwd
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

bool File::open(const std::string& path_, const std::string& mode_){
	path(path_);
	mode(mode_);
	return open();
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
		//printf("reading %d bytes from %s\n", n, path().c_str());
		allocContent(n);
		int numRead = fread(mContent, sizeof(char), n, mFP);
		if(numRead < n){
			//printf("warning: only read %d bytes\n", numRead);
		}
	}
	return mContent;
}

std::string File::read(const std::string& path){
	File f(path, "rb");
	f.open();
	auto str = f.readAll();
	return str ? str : "";
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

bool File::remove(const std::string &path)
{
	if (!File::isDirectory(path)) {
		return ::remove(path.c_str()) == 0;
	}
	return false;
}

bool File::copy(const std::string &srcPath, const std::string &dstPath, unsigned int bufferSize)
{
	std::ifstream src(srcPath, std::ios::binary);	
	if(!src.is_open()){
		return false;
	}

	std::string outPath = dstPath;
	if (File::isDirectory(outPath)) {
		outPath += AL_FILE_DELIMITER_STR + File::baseName(srcPath);
	}
	outPath = File::conformPathToOS(outPath);
	
	std::ofstream dst(outPath, std::ios::binary);
	if(!dst.is_open()){
		return false;
	}

	dst << src.rdbuf();

	return true;
}



std::string File::conformDirectory(const std::string& path){
	if(path[0]){
		if(AL_FILE_DELIMITER != path[path.size()-1]){
			return path + AL_FILE_DELIMITER;
		}
		return path;
	}
	return "." AL_FILE_DELIMITER_STR;
}

std::string File::conformPathToOS(const std::string& path){
	std::string res(path);

	// Ensure delimiters are correct
	for(unsigned i=0; i<path.size(); ++i){
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

std::string File::absolutePath(const std::string& path){
#ifdef AL_WINDOWS
	TCHAR dirPart[4096];
	TCHAR ** filePart={NULL};
	GetFullPathName((LPCTSTR)path.c_str(), sizeof(dirPart), dirPart, filePart);
	std::string result = (char *)dirPart;
	if(filePart != NULL && *filePart != 0){
		result += (char *)*filePart;
	}
	return result;
#else
	char temp[PATH_MAX];
	char * result = realpath(path.c_str(), temp);
	return result ? result : "";
#endif
}

std::string File::baseName(const std::string& path, const std::string& suffix){
	auto posSlash = path.find_last_of("/\\"); // handle '/' or '\' path delimiters
	if(path.npos == posSlash) posSlash=0; // no slash
	else ++posSlash;
	auto posSuffix = suffix.empty() ? path.npos : path.find(suffix, posSlash);
	auto len = path.npos;
	if(path.npos != posSuffix) len = posSuffix - posSlash;
	return path.substr(posSlash, len);
}

std::string File::directory(const std::string& path){
	size_t pos = path.find_last_of(AL_FILE_DELIMITER);
	if(std::string::npos != pos){
		return path.substr(0, pos+1);
	}
	return "." AL_FILE_DELIMITER_STR;
}

std::string File::extension(const std::string& path){
	size_t pos = path.find_last_of('.');
	if(path.npos != pos){
		return path.substr(pos);
	}
	return "";
}

static std::string stripEndSlash(const std::string& path){
	if(path.back() == '\\' || path.back() == '/'){
		return path.substr(0, path.size()-1);
	}
	return path;
}

bool File::exists(const std::string& path){
	struct stat s;
	return ::stat(stripEndSlash(path).c_str(), &s) == 0;
}

bool File::isDirectory(const std::string& path){
	struct stat s;
	if(0 == ::stat(stripEndSlash(path).c_str(), &s)){	// exists?
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


void FileList::print() const {
	printf("FileList:\n");
	for(const auto& f : mFiles)
		printf("%s\n", f.filepath().c_str());
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
	if(platform_getcwd(cwd, sizeof(cwd))){
		mAppPath = std::string(cwd) + "/";
		addSearchPath(mAppPath, recursive);
	}
}

void SearchPaths::addSearchPath(const std::string& src, bool recursive) {
	std::string path=File::conformDirectory(src);

	// check for duplicates
	for(const auto& sp : mSearchPaths){
		//printf("path %s\n", sp.first.c_str());
		if(path == sp.first) return;
	}

	//printf("adding path %s\n", path.data());
	mSearchPaths.emplace_front(path, recursive);
}

// Returns true if one or more matches, otherwise false
template <class PredicateOnFileAndDir>
bool findFile(
	FileList& result,
	const std::string& dir, bool recursive, bool findFirst,
	PredicateOnFileAndDir pred
){
	bool found = false;
	Dir d;
	if(d.open(dir)){
		while(d.read()){
			const auto& entry = d.entry();
			if((entry.type()==FileInfo::DIR) && recursive && (entry.name()[0]!='.')){
				if(findFile(result, dir+entry.name()+AL_FILE_DELIMITER_STR, true, findFirst, pred)){
					if(findFirst) return true;
					found = true;
				}
			}
			else if((entry.type()==FileInfo::REG) && pred(entry.name(), dir)){
				result.add(FilePath(entry.name(), dir));
				if(findFirst) return true;
				found = true;
			}
		}
	}
	return found;
}

FilePath SearchPaths::find(const std::string& file) const {
	FileList fileList;
	for(const auto& searchPath : mSearchPaths){
		if(
			findFile(
				fileList,
				searchPath.first, searchPath.second, true,
				[&file](std::string f, std::string d){ return f==file; }
			)
		){
			return fileList[0];
		}
	}
	return FilePath();
}

FileList SearchPaths::glob(const std::string& regex) const {
	FileList fileList;
	for(const auto& searchPath : mSearchPaths){
		findFile(
			fileList,
			searchPath.first, searchPath.second, false,
			[&regex](std::string f, std::string d){
				return std::regex_match(d+f,std::regex(regex));
			}
		);
	}
	return fileList;
}

void SearchPaths::print() const {
	printf("SearchPath %p appPath: %s\n", this, appPath().c_str());
	for(const auto& sp : mSearchPaths){
		printf("SearchPath %p path: %s (recursive: %d)\n", this, sp.first.c_str(), sp.second);
	}
}


} // al::

