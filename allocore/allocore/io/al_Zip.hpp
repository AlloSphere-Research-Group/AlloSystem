#ifndef INCLUDE_AL_ZIP_HPP
#define INCLUDE_AL_ZIP_HPP
/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Zip file reader/writer

	File author(s):
	Lance Putnam, 2021, putnam.lance@gmail.com
*/

#include <functional>
#include <string>
#include <vector>
#include "allocore/system/al_Pimpl.hpp"

namespace al{

class ZipReader {
public:

	ZipReader();

	/// Open archive file
	bool open(const std::string& path);

	/// Extract file in archive to heap memory
	bool extract(
		const std::string& fileName,
		const std::function<void(const void * data, int size)>& onData
	);

	/// Extract file in zip archive to heap memory

	/// This is the simplest way to extract a single file from an archive.
	/// If extracting multiple files from the same archive, create an object
	/// and call extract for each file.
	static bool extract(
		const std::string& zipPath,
		const std::string& fileName,
		const std::function<void(const void * data, int size)>& onData
	);

	/// Get file paths contained in archive
	const std::vector<std::string>& filePaths() const { return mFilePaths; }

private:
	class Impl;
	Pimpl<Impl> mImpl;

	std::vector<std::string> mFilePaths;
};


class ZipWriter {
public:

	ZipWriter();
	~ZipWriter();

	/// Open archive file
	bool open(const std::string& path);

	/// Finalize and close the archive
	void close();

	/// Add file to archive

	/// @param[in] filePath		Path of file on disk
	/// @param[in] zipPath		Destination file path in archive
	bool addFile(const std::string& filePath, const std::string& zipPath="");

	/// Add directory to archive
	bool addDir(const std::string& zipDir);

	/// Add memory buffer to archive
	bool addMem(const void * buf, int bytes, const std::string& zipPath="");

	/// Set compression level, in 0-10

	/// A value of 0 means no compression (fastest), a value of 10 means max 
	/// compression (slowest) and a value of -1 means default compression.
	ZipWriter& compression(int level = -1);

private:
	class Impl;
	Pimpl<Impl> mImpl;

	int mCompression = -1;
};

} // al::
#endif


