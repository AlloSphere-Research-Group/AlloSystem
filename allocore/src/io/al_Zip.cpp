#include "allocore/io/al_Zip.hpp"
#include "miniz/miniz.h"
#include "miniz/miniz.c"

namespace al{

struct ZipReader::Impl{
	mz_zip_archive zip;

	Impl(){
		mz_zip_zero_struct(&zip);
	}

	~Impl(){
		mz_zip_reader_end(&zip);
	}
};

ZipReader::ZipReader(){
	mImpl = new Impl;
}

ZipReader::~ZipReader(){
	delete mImpl;
}

bool ZipReader::open(const std::string& path){
	auto * zip = &mImpl->zip;
	auto status = mz_zip_reader_init_file(zip, path.c_str(), 0);
	if(status){
		int numFiles = mz_zip_reader_get_num_files(zip);
		mFilePaths.clear();
		char buf[128];
		for(int i=0; i<numFiles; ++i){
			auto bytes = mz_zip_reader_get_filename(zip, i, buf, sizeof buf);
			mFilePaths.emplace_back(buf, bytes);
		}
		return true;
	}
	return false;
}

bool ZipReader::extract(
	const std::string& fileName,
	const std::function<void(const void * data, int size)>& onData
){
	size_t size;
	void * data = mz_zip_reader_extract_file_to_heap(&mImpl->zip, fileName.c_str(), &size, 0);
	if(data){
		//printf("Extracted file (%d kB)\n", size/1000); fflush(stdout);
		onData(data, size);
		mz_free(data);
		return true;
	}
	return false;
}

/*static*/ bool ZipReader::extract(
	const std::string& zipPath,
	const std::string& fileName,
	const std::function<void(const void * data, int size)>& onData
){
	ZipReader zip;
	return zip.open(zipPath) && zip.extract(fileName, onData);
}

} // al::