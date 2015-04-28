 /*

   sliceassembler

   Voxel data import aka slice assembler aka volume reconstructor 

   by Matt Wright, April 2015

   based on tiExporter.cpp from images2raw.cpp by Coby Kaufer
   <cobykaufer@bluejayke.com>, Karl Yerkes <karl.yerkes@gmail.com>,
   and Matt Wright <matt@create.ucsb.edu

   Read in a directory full of 2D image files with some naming
   convention, assemble them all into an al::Array or al::Voxels
   and write the result as one huge fast-to-load raw binary data file.


   Limitations:

   - Images must contain 8-bit RGB pixels

   - Ignores all but the red channel

   - Ignores "info.txt" file: assumes 1nm cube voxels  rather than reading in the voxel physical
     size 

   - Ignores "info.txt" file, not verifying the stated number of images or image resolutions.

   - Chokes if directory contains anything besides "info.txt" and image files

   - Does nothing useful after reading in all this data


   Usage:

   You pass in the directory name as the command-line argument, which means that the
   run.sh script can't invoke the program correctly, so build and run as separate steps:

   ./run.sh  allocore/examples/spatial/sliceassembler.cpp
   [Ignore error message from invoking program with no command-line arguments]

   ./build/bin/allocore_examples_spatial_sliceassembler  ~/repos/spherical_harmonic_generator/example/sample10_a

*/

#include <dirent.h>
#include <cassert>
#include <iostream>
#include "allocore/al_Allocore.hpp"
#include "allocore/types/al_Voxels.hpp"
// #include <conio.h>

using namespace al;
using namespace std;

#ifdef FASTLOAD
// Some way of skipping slices or taking just a corner of the volume or...???
#endif


int getdir(string dir, vector<string> &files) {
  DIR *dp;
  struct dirent * dirp;
  if((dp = opendir(dir.c_str())) == NULL)	{
    //cout << "Error(" << errno << ") opening " << dir << endl;
    return 1;
  }
  while((dirp = readdir(dp)) != NULL) {
    char *name = dirp->d_name;
    if (strcmp(name, ".") && strcmp(name, "..") && strcmp(name, "info.txt")) {
      files.push_back(dir + "/" + string(name));
    }
  }
  closedir(dp);
  return 0;
}


int main(int argc, char *argv[]) {

  // Parse args...
  if (argc != 3) {
    cout << "You suck.  Give a directory and output filename"  << endl;
    exit(-3);
  }

  string dir(argv[1]);
  vector<string> files;

  // Image and Texture handle reading and displaying image files.
  Image RGBImage; // for reading into
  
  if (getdir(dir,files) != 0) {
    cout << "Problem reading directory " << dir << endl;
    exit(-1);
  }

  if (files.size() == 0) {
    cout << "Read zero files from directory " << dir << endl;
    exit(-2);
  }

  cout << "Judging by " << dir << " there are " << files.size() << " images (or at least files)" << endl;


  // Try reading the first one just to get the size
  if (!RGBImage.load(files[0])) {
    cout << "Couldn't read file " << files[0] << endl;
    exit(-3);
  }

  int nx = RGBImage.width();
  int ny = RGBImage.height();
  int nz = files.size();

  cout << "Judging by " << files[0] << " each image should be " << nx << " by " <<
    ny << endl;

  // For now assume 8-bit with 1 nm cube voxels
  Voxels v(AlloUInt8Ty, nx, ny, nz, 1., 1., 1., NANOMETERS);
  
  v.print();

  // Iterate through entire directory
  int slice = 0;
  for (vector<string>::iterator it = begin (files); it != end (files); ++it, ++slice) {
    string &filename = *it;

    if (RGBImage.load(filename)) {
      cout << "loaded " << filename << 
        " (" << slice+1 << " of " << files.size() << ")" << endl;
    } else {
      cout << "Failed to read image from " << filename << endl;
      exit(-4);
    }

    // Verify XY resolution
    if (RGBImage.width() != nx || RGBImage.height() != ny) {
      cout << "Error:  resolution mismatch!" << endl;
      cout << "   " << files[0] << ": " << nx << " by " << ny << endl;
      cout << "   " << filename << ": " << RGBImage.width() << " by " << RGBImage.height() << endl;
      exit(-5);
    }
      

    // Access the read-in image data
    Array& array(RGBImage.array());
    
    // For now assume 8-bit RGBA
    Image::RGBAPix<uint8_t> pixel;

    // Copy it out pixel-by-pixel:
    for (size_t row = 0; row < array.height(); ++row) {
      for (size_t col = 0; col < array.width(); ++col) {
        array.read(&pixel, col, row);

        // For now we'll take only the red and put it in the single component; that's lame.
        v.elem<char>(1, col, row, slice) = (char) pixel.r;
      }
    }
  }
  // v is ready


  // how to write raw data without using default voxel file IO system
  //  const char *outputFile = "VolumeData.raw";
  //  FILE *fp=fopen(outputFile, "w");
  //  if (fp == NULL) {
  //    cout << "error " << errno << " opening " << outputFile << " for writing " << endl;
  //    exit(-5);
  //  }
  //
  //  char * data =  v.cell<char>(0);
  //
  //  fwrite(data, sizeof(char), nx * ny * nz, fp);
  //  fclose(fp);
  
  v.writeToFile(argv[2]);
}
