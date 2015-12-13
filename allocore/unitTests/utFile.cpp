#include "utAllocore.h"

#include "catch.hpp"

TEST_CASE( "File", "[file]" ) {

	#define DELIM AL_FILE_DELIMITER_STR

	// static functions
	REQUIRE(File::exists("."));
	//REQUIRE(File::exists("." DELIM)); // fails under win32

	REQUIRE(File::isDirectory("."));
	REQUIRE(File::isDirectory(".." ));
	REQUIRE(!File::isDirectory(".." DELIM ".." DELIM "Makefile"));

	REQUIRE(
		File::conformDirectory("test")
		== "test" DELIM
	);
	REQUIRE(
		File::conformDirectory("test" DELIM)
		== "test" DELIM
	);

	REQUIRE(
		File::conformPathToOS("..\\../Makefile")
		== ".." DELIM ".." DELIM "Makefile"
	);

	REQUIRE(
		File::conformPathToOS("..\\../")
		== ".." DELIM ".." DELIM
	);

	REQUIRE(
		File::conformPathToOS("..\\..")
		== ".." DELIM ".." DELIM
	);

	// simple file/directory searching
	{
		std::string dir;

		// Search for file (that exists and probably will not be moved...)
		std::string find = "Makefile";

		bool r = File::searchBack(dir, find);
		REQUIRE(r);
		REQUIRE(File::exists(dir + find));

		find = "allocore" DELIM + find;	// check for a file with path
		r = File::searchBack(dir, find);
		REQUIRE(r);
		REQUIRE(File::exists(dir + find));

		REQUIRE(!File::searchBack(dir, "thisdirectorydoesnotexist" DELIM "thisfiledoesnotexist.ext"));
	}

	{
		const char * path = "utFile.txt";
		const char * text = "This is a test of AlloCore file i/o functionality. You can safely delete this file.";

		// write data
		File f(path, "w");
		REQUIRE(!f.opened());

		REQUIRE(f.open());
		REQUIRE(f.opened());
		REQUIRE(File::exists(path));

		REQUIRE(
			f.write(text, strlen(text))
		);

		REQUIRE(f.size() == (int)strlen(text));

		f.close();
		REQUIRE(!f.opened());
		REQUIRE(File::exists(path));


		// read data
		f.mode("r").open();
		REQUIRE(f.opened());

		const char * read = f.readAll();

		REQUIRE(f.size() == (int)strlen(text));

		for(int i=0; i<f.size(); ++i){
			REQUIRE(read[i] == text[i]);
//			printf("%c", read[i]);
		}

		f.close();

		f.path(
			"thisdirectroydoesnotexist" DELIM
			"neitherdoesthisone" DELIM
			"notafile.txt"
		);
		REQUIRE(!f.open());
	}


	{
		REQUIRE(Dir::make("utFileTestDir"));
		REQUIRE(Dir::remove("utFileTestDir"));

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
}
