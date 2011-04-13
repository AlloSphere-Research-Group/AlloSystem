#include "utAllocore.h"

int utFile() {

	// static functions
	assert(fileExists("./"));
	
	{
		const char * path = "utFile.txt";
		const char * text = "This is a test of AlloCore file i/o functionality.\nYou can safely delete this file.";

		// write data
		File f(path, "w");
		assert(!f.opened());

		assert(f.open());
		assert(f.opened());
		assert(fileExists(path));

		assert(
			f.write(text, strlen(text))
		);
		
		assert(f.size() == (int)strlen(text));

		f.close();
		assert(!f.opened());
		assert(fileExists(path));


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
		
		f.path("thisdirectroydoesnotexist//neitherdoesthisone//notafile.txt");
		assert(!f.open());
	}
	
	
	{
		SearchPaths sp;
		//printf("%s\n", sp.appPath().c_str());
	}
	
	{
		FilePath fp("file.txt", "path");

		//printf("%s\n", fp.filepath().c_str());
	}
	
	return 0;
}
