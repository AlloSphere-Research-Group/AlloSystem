#include <iostream>
#include "allocore/io/al_File.hpp"

using namespace al;

int main() {
	SearchPaths searchPaths;
	searchPaths.addSearchPath(".");

	// matches .png .jpg files and adds them to a FileList
	FileList files = searchPaths.glob("(.*)\\.(png|jpg)");
	std::cout << "Matched file count: " << files.count() << std::endl;

	// sort files by filepath
	files.sort([](FilePath a, FilePath b){
		return a.filepath() < b.filepath();
	});

	files.print();
}
