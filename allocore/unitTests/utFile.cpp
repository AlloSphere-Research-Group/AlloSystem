#include <string.h>
#include "utAllocore.h"
#include "allocore/io/al_File.hpp"

int utFile() {
	using namespace al;

	#define DELIM AL_FILE_DELIMITER_STR

	// static functions
	assert(File::exists("."));
	//assert(File::exists("." DELIM)); // fails under win32

	assert(File::isDirectory("."));
	assert(File::isDirectory(".." ));
	assert(!File::isDirectory(".." DELIM ".." DELIM "Makefile"));

	assert(
		File::conformDirectory("test")
		== "test" DELIM
	);
	assert(
		File::conformDirectory("test" DELIM)
		== "test" DELIM
	);

	assert(
		File::conformPathToOS("..\\../Makefile")
		== ".." DELIM ".." DELIM "Makefile"
	);

	assert(
		File::conformPathToOS("..\\../")
		== ".." DELIM ".." DELIM
	);

	assert(
		File::conformPathToOS("..\\..")
		== ".." DELIM ".." DELIM
	);

	// simple file/directory searching
	{
		std::string dir;

		// Search for file (that exists and probably will not be moved...)
		std::string find = "Makefile";

		bool r = File::searchBack(dir, find);
		assert(r);
		assert(File::exists(dir + find));

		find = "allocore" DELIM + find;	// check for a file with path
		r = File::searchBack(dir, find);
		assert(r);
		assert(File::exists(dir + find));

		assert(!File::searchBack(dir, "thisdirectorydoesnotexist" DELIM "thisfiledoesnotexist.ext"));
	}

	{
		const char * path = "utFile.txt";
		const char * text = "This is a test of AlloCore file i/o functionality. You can safely delete this file.";

		// write data
		File f(path, "w");
		assert(!f.opened());

		assert(f.open());
		assert(f.opened());
		assert(File::exists(path));

		assert(
			f.write(text, strlen(text))
		);

		assert(f.size() == (int)strlen(text));

		f.close();
		assert(!f.opened());
		assert(File::exists(path));


		// read data
		f.mode("r").open();
		assert(f.opened());

		const char * read = f.readAll();

		assert(f.size() == (int)strlen(text));

		for(int i=0; i<f.size(); ++i){
			assert(read[i] == text[i]);
//			printf("%c", read[i]);
		}

		f.close();

		f.path(
			"thisdirectroydoesnotexist" DELIM
			"neitherdoesthisone" DELIM
			"notafile.txt"
		);
		assert(!f.open());
	}


	{
		assert(Dir::make("utFileTestDir"));
		assert(Dir::remove("utFileTestDir"));

		// For now, just make sure this compiles and doesn't crash. :)
		Dir dir;
		if(dir.open("/")){
			for(int i=0; i<1; ++i){
				dir.rewind();
				while(dir.read()){
					//printf("%c %s\n", dir.entry().type() == FileInfo::DIR ? 'd':' ', dir.entry().name().c_str());
				}
			}
		}
	}

	{
		// TODO:
		SearchPaths sp;
		//printf("%s\n", sp.appPath().c_str());
	}

	{
		// TODO:
		FilePath fp("file.txt", "path");

		//printf("%s\n", fp.filepath().c_str());
	}

	#undef DELIM
	return 0;
}
