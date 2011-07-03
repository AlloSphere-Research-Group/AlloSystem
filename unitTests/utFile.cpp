#include "utAllocore.h"

int utFile() {

	// static functions
	assert(File::exists("./"));

	// simple file/directory searching
	{
		std::string dir;
		
		// Search for file (that exists and probably will not be moved...)
		std::string find = "Makefile";
		
		bool r = File::searchBack(dir, find);
		assert(r);
		assert(File::exists(dir + find));
		
		find = ".."AL_FILE_DELIMITER_STR + find;	// check for a file with path
		r = File::searchBack(dir, find);
		assert(r);
		assert(File::exists(dir + find));

		assert(!File::searchBack(dir, "thisdirectorydoesnotexist"AL_FILE_DELIMITER_STR"thisfiledoesnotexist.ext"));
	}

	{
		const char * path = "utFile.txt";
		const char * text = "This is a test of AlloCore file i/o functionality.\nYou can safely delete this file.";

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
		
		f.path("thisdirectroydoesnotexist"AL_FILE_DELIMITER_STR"neitherdoesthisone"AL_FILE_DELIMITER_STR"notafile.txt");
		assert(!f.open());
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
	
	return 0;
}
