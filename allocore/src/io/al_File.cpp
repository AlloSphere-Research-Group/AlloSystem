#include <cstring>
#include <fstream> // ifstream, ofstream
#include <memory>
#include <regex>
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Config.h"

using namespace al;


static std::string stripEndSlash(const std::string& path){
	if(path.back() == '\\' || path.back() == '/'){
		return path.substr(0, path.size()-1);
	}
	return path;
}


#if defined(AL_OSX) || defined(AL_LINUX) || defined(AL_EMSCRIPTEN)
#include <cstdlib> // getenv
#include <dirent.h>
#include <unistd.h> // rmdir, getcwd
#include <stdlib.h> // realpath
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // DEV_BSIZE
#include <errno.h>

al_sec File::modified() const {
	struct stat s;
	if(::stat(path().c_str(), &s) == 0){
		//const auto& t = s.st_mtim;
		//return t.tv_sec + t.tv_usec/1e9;
		return s.st_mtime;
	}
	return 0.;
}

al_sec File::accessed() const {
	struct stat s;
	if(::stat(path().c_str(), &s) == 0){
		return s.st_atime;
	}
	return 0.;
}

al_sec File::created() const {
	// Not supported on POSIX systems (st_ctime is last status change):
	// https://lwn.net/Articles/397442/
	return 0.;
}

size_t File::sizeFile() const {
	struct stat s;
	if(::stat(path().c_str(), &s) == 0){
		return s.st_size;
	}
	return 0;
}

size_t File::storage() const {
	struct stat s;
	if(::stat(path().c_str(), &s) == 0){
		return s.st_blocks*DEV_BSIZE;
	}
	return 0;
}

/*static*/ std::string File::absolutePath(const std::string& path){
	char temp[PATH_MAX];
	char * result = realpath(path.c_str(), temp);
	return result ? result : "";
}

/*static*/ bool File::exists(const std::string& path){
	struct stat s;
	return ::stat(stripEndSlash(path).c_str(), &s) == 0;
}

/*static*/ bool File::isDirectory(const std::string& path){
	struct stat s;
	if(0 == ::stat(stripEndSlash(path).c_str(), &s)){	// exists?
		if(s.st_mode & S_IFDIR){		// is dir?
			return true;
		}
	}
	// if(s.st_mode & S_IFREG) // is file?
	return false;
}


class Dir::Impl{
public:
	DIR * mDir = NULL;
	struct dirent * mEnt = NULL;
	std::string mPath;

	bool open(const std::string& path){
		if((mDir = opendir(path.c_str())) != NULL){
			mPath = path;
			return true;
		}
		return false;
	}

	bool close(){
		if(mDir && closedir(mDir) == 0){
			mPath = "";
			return true;
		}
		return false;
	}

	bool read(FileInfo& fileInfo){
		if((mEnt = readdir(mDir)) != NULL){

			fileInfo.name(mEnt->d_name);

			#if defined(_DIRENT_HAVE_D_TYPE) || defined(AL_OSX)
			switch(mEnt->d_type){
				case DT_REG: fileInfo.type(FileInfo::REG); break;
				case DT_DIR: fileInfo.type(FileInfo::DIR); break;
				case DT_CHR: fileInfo.type(FileInfo::CHR); break;
				case DT_BLK: fileInfo.type(FileInfo::BLOCK); break;
				case DT_LNK: fileInfo.type(FileInfo::LINK); break;
				case DT_SOCK:fileInfo.type(FileInfo::SOCKET); break;
				case DT_FIFO: fileInfo.type(FileInfo::PIPE); break;
				default: fileInfo.type(FileInfo::UNKNOWN); break;
			}
			#else // fallback that uses much slower stat command
			struct stat s;
			if(::stat((mPath + "/" + fileInfo.name()).c_str(), &s) == 0){
				switch(s.st_mode & S_IFMT){
					case S_IFREG: fileInfo.type(FileInfo::REG); break;
					case S_IFDIR: fileInfo.type(FileInfo::DIR); break;
					case S_IFCHR: fileInfo.type(FileInfo::CHR); break;
					case S_IFBLK: fileInfo.type(FileInfo::BLOCK); break;
					case S_IFLNK: fileInfo.type(FileInfo::LINK); break;
					case S_IFSOCK:fileInfo.type(FileInfo::SOCKET); break;
					case S_IFIFO: fileInfo.type(FileInfo::PIPE); break;
					default: fileInfo.type(FileInfo::UNKNOWN); break;
				}
			}
			#endif

			return true;
		}
		return false;
	}

	bool rewind(){
		if(mDir) rewinddir(mDir);
		return true;
	}
};

/*static*/ bool Dir::make(const std::string& path, bool recursive){
	auto mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	if(recursive){
		// Create all intermediate dirs up to last one
		for(unsigned i=0; i<path.size(); ++i){
			if(path[i] == '/'){
				if(mkdir(path.substr(0, i+1).c_str(), mode) != 0){
					if(errno != EEXIST) return false;
				}
			}
		}
	}

	return mkdir(path.c_str(), mode) == 0;
}

/*static*/ bool Dir::remove(const std::string& path){
	return rmdir(path.c_str()) == 0;
}

/*static*/ std::string Dir::cwd(){
	char buf[AL_PATH_MAX];
	if(getcwd(buf, sizeof(buf))){
		return std::string(buf);
	}
	return "";
}

static std::string envDir(const char * envVar){
	char * s = std::getenv(envVar);
	return s ? std::string(s) + "/" : std::string();
}

/*static*/ std::string Dir::home(){ return envDir("HOME"); }

#if defined(AL_OSX)
/*static*/ std::string Dir::appData(){ return "/Library/Application Support/"; }
/*static*/ std::string Dir::userAppData(){ return home()+"Library/Application Support/"; }

#elif defined(AL_LINUX) || defined(AL_EMSCRIPTEN)
/*static*/ std::string Dir::appData(){ return "/var/lib/"; }
/*static*/ std::string Dir::userAppData(){ return home()+".config/"; }
#endif

// end POSIX


#elif defined(AL_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <Shlwapi.h> // PathFileExists, PathIsDirectory
#pragma comment(lib, "Shlwapi.lib")
#include <Shlobj.h> // SHGetFolderPath, SHGetKnownFolderPath
#pragma comment(lib,"Shell32.lib") // SHGetKnownFolderPath 
#pragma comment(lib,"Ole32.lib") // CoTaskMemFree
#pragma comment(lib,"uuid.lib") // FOLDERID_ProgramData

#if defined(_MSC_VER) || defined(__MINGW32__)
	#include <direct.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

struct ReadOnlyFileHandle{
	HANDLE mHandle = INVALID_HANDLE_VALUE;
	ReadOnlyFileHandle(const std::string& path){
		mHandle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	}
	~ReadOnlyFileHandle(){
		if(opened()) CloseHandle(mHandle);
	}
	bool opened() const { return mHandle != INVALID_HANDLE_VALUE; }
};

unsigned long long toUInt64(DWORD lo, DWORD hi){
	ULARGE_INTEGER t;
	t.LowPart = lo;
	t.HighPart = hi;
	return t.QuadPart;
}

al_sec fileTimeToSec(const FILETIME& fileTime){
	// File times in 100-nanosecond intervals that have elapsed since
	// 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
	// Need to return seconds since Jan 1, 1970. The difference is 134774 days.
	// https://msdn.microsoft.com/en-us/library/ms724290(v=vs.85).aspx
	return toUInt64(fileTime.dwLowDateTime, fileTime.dwHighDateTime) * 1e-7 - 11644473600.;
}

al_sec File::modified() const {
	ReadOnlyFileHandle fh(path());
    if(fh.opened()){
		FILETIME ftCreate, ftAccess, ftWrite;
		if(GetFileTime(fh.mHandle, &ftCreate, &ftAccess, &ftWrite)){
			return fileTimeToSec(ftWrite);
		}
	}
	return 0.;
}

al_sec File::accessed() const {
	ReadOnlyFileHandle fh(path());
    if(fh.opened()){
		FILETIME ftCreate, ftAccess, ftWrite;
		if(GetFileTime(fh.mHandle, &ftCreate, &ftAccess, &ftWrite)){
			return fileTimeToSec(ftAccess);
		}
	}
	return 0.;
}

al_sec File::created() const {
	ReadOnlyFileHandle fh(path());
    if(fh.opened()){
		FILETIME ftCreate, ftAccess, ftWrite;
		if(GetFileTime(fh.mHandle, &ftCreate, &ftAccess, &ftWrite)){
			return fileTimeToSec(ftCreate);
		}
	}
	return 0.;
}

size_t File::sizeFile() const {
	ReadOnlyFileHandle fh(path());
    if(fh.opened()){
		/*BY_HANDLE_FILE_INFORMATION info;
		if(GetFileInformationByHandle(fh.mHandle, &info)){
			return toUInt64(info.nFileSizeLow, info.nFileSizeHigh);
		}*/
		return GetFileSize(fh.mHandle, NULL);
	}
	return 0;
}

size_t File::storage() const {
	return GetCompressedFileSizeA(path().c_str(), NULL);
}

/*static*/ std::string File::absolutePath(const std::string& path){
	TCHAR dirPart[4096];
	TCHAR ** filePart={NULL};
	GetFullPathNameA(path.c_str(), sizeof(dirPart), dirPart, filePart);
	std::string result = (char *)dirPart;
	if(filePart != NULL && *filePart != 0){
		result += (char *)*filePart;
	}
	return result;
}

/*static*/ bool File::exists(const std::string& path){
	return PathFileExistsA(path.c_str());
}

/*static*/ bool File::isDirectory(const std::string& path){
	return PathIsDirectoryA(path.c_str());
}


class Dir::Impl{
public:

	bool open(const std::string& path){
		mPath = path;
		return rewind();
	}

	bool close(){
		if(opened()){
			return FindClose(mHandle) != 0;
		}
		return false;
	}

	bool read(FileInfo& fileInfo){
		if(opened()){
			if(mFindNextFile){
				if(FindNextFile(mHandle, &mSearchData) == 0){
					return false;
				}
			}

			auto attr = mSearchData.dwFileAttributes;
			//printf("dwFileAttributes: %d\n", attr);
			if(attr & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE)){
				fileInfo.type(FileInfo::REG);
			}
			else if(attr & FILE_ATTRIBUTE_DIRECTORY){
				fileInfo.type(FileInfo::DIR);
			}
			else if(attr & FILE_ATTRIBUTE_REPARSE_POINT){
				fileInfo.type(FileInfo::LINK);
			}
			else{
				fileInfo.type(FileInfo::UNKNOWN);
			}

			fileInfo.name(mSearchData.cFileName);

			mFindNextFile = true;
			return true;
		}
		return false;
	}

	bool rewind(){
		auto searchPath = mPath + "\\*"; // must append wildcard to list all entries
		mHandle = FindFirstFile(searchPath.c_str(), &mSearchData);
		mFindNextFile = false;
		return opened();
	}

	bool opened() const { return mHandle != INVALID_HANDLE_VALUE; }

private:
	std::string mPath;
	HANDLE mHandle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA mSearchData;
	bool mFindNextFile = false;
};

/*static*/ bool Dir::make(const std::string& path, bool recursive){
	if(recursive){
		// Create all intermediate dirs up to last one
		for(unsigned i=0; i<path.size(); ++i){
			if(path[i] == '/'){
				if(CreateDirectory(path.substr(0, i+1).c_str(), NULL) != 0){
					if(GetLastError() != ERROR_ALREADY_EXISTS) return false;
				}
			}
		}
	}

	return CreateDirectory(path.c_str(), NULL) != 0;
}

/*static*/ bool Dir::remove(const std::string& path){
	if(RemoveDirectory(path.c_str()) != 0){
		return true;
	}
	return false;
}

/*static*/ std::string Dir::cwd(){
	char buf[AL_PATH_MAX];
	if(GetCurrentDir(buf, sizeof(buf))){
		return std::string(buf);
	}
	return "";
}

static std::string getKnownDir(REFKNOWNFOLDERID id){
	PWSTR wdir = nullptr;
	std::string dir;
	if(SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &wdir))){
		char c[4 * MAX_PATH];
		int len = WideCharToMultiByte(CP_UTF8, 0, wdir, lstrlenW(wdir), c, sizeof(c), nullptr, nullptr);
		dir = std::string(c, len) + "\\";
	}
	CoTaskMemFree(wdir);
	return dir;

	/*
	SHFOLDERAPI SHGetFolderPathA(
	  HWND   hwnd,
	  int    csidl,
	  HANDLE hToken,
	  DWORD  dwFlags,
	  LPSTR  pszPath
	);
	*/
	/*TCHAR dir[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, dir))){
		//https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
		//using convert_type = std::codecvt_utf8<wchar_t>;
		//std::wstring_convert<convert_type, wchar_t> converter;
		//return converter.to_bytes(std::wstring(dir));
		return std::string(dir);
	}*/
}

/*static*/ std::string Dir::appData(){ return getKnownDir(FOLDERID_ProgramData); }
/*static*/ std::string Dir::userAppData(){ return getKnownDir(FOLDERID_LocalAppData); }
/*static*/ std::string Dir::home(){ return getKnownDir(FOLDERID_Profile); }

// End Windows


#else // Unsupported platform

al_sec File::modified() const {	return 0.; }
al_sec File::accessed() const {	return 0.; }
al_sec File::created() const { return 0.; }
size_t File::sizeFile() const {	return 0; }
size_t File::storage() const { return 0; }
/*static*/ std::string File::absolutePath(const std::string& path){ return path; }
/*static*/ bool File::exists(const std::string& path){ return false; }
/*static*/ bool File::isDirectory(const std::string& path){	return false; }
class Dir::Impl{
public:
	bool open(const std::string& path){ return false; }
	bool close(){ return false; }
	bool read(FileInfo& fileInfo){ return false; }
	bool rewind(){ return true; }
};
/*static*/ bool Dir::make(const std::string& path, bool recursive){ return false; }
/*static*/ bool Dir::remove(const std::string& path){ return false; }
/*static*/ std::string Dir::cwd(){ return "."; }
/*static*/ std::string Dir::appData(){ return {}; }
/*static*/ std::string Dir::userAppData(){ return {}; }
/*static*/ std::string Dir::home(){ return {}; }

#endif


// Platform independent code

File::File(const std::string& path, const std::string& mode, bool open_)
:	mPath(path), mMode(mode), mContent(0), mSizeBytes(0), mFP(0)
{	if(open_) open(); }

File::File(const FilePath& path, const std::string& mode, bool open_)
:	File(path.filepath(), mode, open_)
{}

File::~File(){
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

static char getDelimiter(const std::string& path){
	auto pos = path.find_first_of("\\/");
	if(pos != std::string::npos){
		return path[pos];
	}
	return AL_FILE_DELIMITER;
}

std::string File::conformDirectory(const std::string& path){
	if(path.size()){
		auto delim = getDelimiter(path);
		if(delim != path[path.size()-1]){
			return path + delim;
		}
		return path;
	}
	return "." AL_FILE_DELIMITER_STR;
}

std::string File::conformPathToOS(const std::string& path){
	std::string res(path);

	#ifdef __MSYS__
	// Replace paths like /c and /c/mydir with c: and c:/mydir
	if(	(res.size() == 2 && '/'==res[0]) ||
		(res.size() >= 3 && '/'==res[0] && '/'==res[2])
	){
		res[0] = res[1];
		res[1] = ':';
	}
	#endif

	// Ensure delimiters are correct
	for(auto& c : res){
		if('\\'==c || '/'==c) c = AL_FILE_DELIMITER;
	}

	// Ensure that directories end with a delimiter
	if(isDirectory(res)){
		return conformDirectory(res);
	}
	return res;
}

std::string File::baseName(const std::string& path, const std::string& suffix){
	auto posSlash = path.find_last_of("/\\"); // handle '/' or '\' path delimiters
	if(path.npos == posSlash) posSlash=0; // no slash
	else ++posSlash;
	//auto posSuffix = suffix.empty() ? path.npos : path.find(suffix, posSlash);
	auto posSuffix = path.npos;
	if(suffix.size()){
		auto wildPos = suffix.find('*');
		posSuffix = path.find(suffix.substr(0, wildPos), posSlash);
	}
	auto len = path.npos;
	if(path.npos != posSuffix) len = posSuffix - posSlash;
	return path.substr(posSlash, len);
}

std::string File::directory(const std::string& path){
	size_t pos = path.find_last_of("/\\");
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

std::string File::replaceExtension(const std::string& path, const std::string& ext){
	if(ext.empty()){
		return path.substr(0, path.find_last_of("."));
	}
	auto e = ext;
	if('.' != e[0]) e = '.' + e;
	return directory(path) + baseName(path, extension(path)) + e;
}

bool File::searchBack(std::string& prefixPath, const std::string& matchPath, int maxDepth){
	auto pre = prefixPath;
	if(pre[0]) pre = conformDirectory(pre);
	for(int i=0; i<maxDepth; ++i){
		if(File::exists(pre + matchPath)){
			prefixPath = pre;
			return true;
		}
		pre += ".." AL_FILE_DELIMITER_STR;
	}
	return false;
}

bool File::searchBack(std::string& path, int maxDepth){
	std::string prefix = "";
	bool r = searchBack(prefix, path);
	if(r) path = prefix + path;
	return r;
}


Dir::Dir()
:	mImpl(new Impl)
{}

Dir::Dir(const std::string& dirToOpen)
:	Dir()
{
	open(dirToOpen);
}

Dir::~Dir(){
	close();
	delete mImpl;
}

bool Dir::open(const std::string& dirPath){ return mImpl->open(dirPath); }

bool Dir::close(){ return mImpl->close(); }

bool Dir::rewind(){ return mImpl->rewind(); }

bool Dir::read(){
	if(mImpl->read(mEntry)){
		const auto& name = mEntry.name();
		switch(mHiddenPolicy){
		default: // exclude hidden
			if(name[0]=='.') return read();
		case 1: // include hidden, except . and ..
			if(name=="." || name=="..") return read();
		case 2: // include hidden or fall-through
			return true;
		}
	}
	return false;
}

/*static*/ bool Dir::removeRecursively(const std::string& path){
	Dir dir(path);
	dir.almostAll(true); // all entries, except . and ..
	bool ok = true;
	while(dir.read()) {
		const auto& entry = dir.entry();
		if(entry.isDir()){
			if(!Dir::removeRecursively(path + "/" + entry.name())){
				ok = false;
				break;
			}
		} else if(entry.isReg()){
			if(!File::remove(path + "/" + entry.name())) {
				ok = false;
				break;
			}
		}
	}
	if(!Dir::remove(path)){
		ok = false;
	}
	return ok;
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
	auto cwd = Dir::cwd();
	if(!cwd.empty()){
		mAppPath = cwd + AL_FILE_DELIMITER_STR;
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
	d.all(true);
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
