#include "utAllocore.h"

int utFile() {
	
	{
		const char * path = "utFile.txt";
		const char * text = "This is a test of AlloCore file i/o functionality.\nYou can safely delete this file.";

		File f(path, "w");
		assert(!f.opened());

		f.open();
		printf("%s\n", f.path());
		assert(f.opened());
		printf("%s\n", f.path());
		assert(File::exists(path));
		printf("%s\n", f.path());

		assert(
			f.write(text, strlen(text))
		);

		f.close();
		assert(!f.opened());
		assert(File::exists(path));

		f.mode("r").open();
		assert(f.opened());
		
		char * read = f.readAll();
		
		assert(f.size() == strlen(text));
		
		for(unsigned i=0; i<f.size(); ++i){
			assert(read[i] == text[i]);
			//printf("%c", read[i]);
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
