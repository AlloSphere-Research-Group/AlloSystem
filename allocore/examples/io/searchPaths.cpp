
#include <iostream>
#include "allocore/io/al_File.hpp"

using namespace al;
using namespace std;

// for sorting the FileList
bool sort(FilePath a, FilePath b){ return a.filepath() < b.filepath();}

int main() {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(".");

  // matches .png .jpg files and adds them to a FileList
  FileList files = searchPaths.glob("(.*)\\.(png|jpg)");
  cout << "Matched file count: " << files.count() << endl;
  files.sort(sort); // sort files by filepath
  files.print();
}
