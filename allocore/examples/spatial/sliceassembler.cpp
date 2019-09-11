/*

   sliceassembler

   Voxel data import aka slice assembler aka volume reconstructor

   by Matt Wright, April 2015
   modified by Hannah Wolfe, July 2015

   based on tiExporter.cpp from images2raw.cpp by Coby Kaufer
   <cobykaufer@bluejayke.com>, Karl Yerkes <karl.yerkes@gmail.com>,
   and Matt Wright <matt@create.ucsb.edu

   Read in a directory full of 2D image files with some naming
   convention, assemble them all into an al::Array or al::Voxels
   and write the result as one huge fast-to-load raw binary data file.


   Limitations:

   - Images must contain 8-bit RGB pixels

   - Ignores all but the red channel

   - Chokes if directory contains anything besides "info.txt" and image files

   - Does nothing useful after reading in all this data

   Usage:

   You pass in the directory name as the command-line argument, which means that
   the
   run.sh script can't invoke the program correctly, so build and run as
   separate steps:

   ./run.sh  allocore/examples/spatial/sliceassembler.cpp
   [Ignore error message from invoking program with no command-line arguments]

   ./build/bin/allocore_examples_spatial_sliceassembler
   ~/repos/spherical_harmonic_generator/example/sample10_a

*/

#include <dirent.h>
#include <cassert>
#include <iostream>
#include <fstream>
//#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_App.hpp"
#include "allocore/types/al_Voxels.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
// #include <conio.h>

using namespace al;
using namespace std;

#ifdef FASTLOAD
// Some way of skipping slices or taking just a corner of the volume or...???
#endif

int getdir(string dir, vector<string> &files) {
  DIR *dp;
  struct dirent *dirp;
  if ((dp = opendir(dir.c_str())) == NULL) {
    // cout << "Error(" << errno << ") opening " << dir << endl;
    return 1;
  }
  while ((dirp = readdir(dp)) != NULL) {
    char *name = dirp->d_name;
    if (strcmp(name, ".") && strcmp(name, "..") && strcmp(name, "info.txt") &&
        strcmp(name, ".DS_Store")) {
      cout << name << endl;
      files.push_back(dir + "/" + string(name));
    }
  }
  closedir(dp);
  return 0;
}

int parseInfo(string dir, vector<string> &data) {
  string file = dir + "/info.txt";
  cout << file << endl;
  ifstream infile(file.c_str());
  if (!infile.good()) return 1;  // exit if file not found

  string strOneLine;

  while (infile) {
    getline(infile, strOneLine);
    if (strOneLine.length() > 0) {
      data.push_back(
          strOneLine.substr(strOneLine.find(":") + 2, strOneLine.length()));
    }
  }

  infile.close();

  return 0;
}

Voxels voxelsFromDirectory(string dir) {
  vector<string> files;
  vector<string> info;

  // Image and Texture handle reading and displaying image files.
  Image RGBImage;  // for reading into

  if (getdir(dir, files) != 0) {
    cout << "Problem reading directory " << dir << endl;
    exit(-1);
  }

  if (files.size() == 0) {
    cout << "Read zero files from directory " << dir << endl;
    exit(-2);
  }

  cout << "Judging by " << dir << " there are " << files.size()
       << " images (or at least files)" << endl;

  // Try reading the first one just to get the size
  if (!RGBImage.load(files[0])) {
    cout << "Couldn't read file " << files[0] << endl;
    exit(-3);
  }

  int nx = RGBImage.width();
  int ny = RGBImage.height();
  int nz = files.size();
  float vx = 1.;
  float vy = 1.;
  float vz = 1.;
  float type = VOX_NANOMETERS;

  if (parseInfo(dir, info) == 0) {
    if (info.size() == 4) {
      type = atoi(info[0].c_str());
      vx = atof(info[1].c_str());
      vy = atof(info[2].c_str());
      vz = atof(info[3].c_str());
      cout << "imported values from info.txt: " << type << ", " << vx << ", "
           << vy << ", " << vz << endl;
    } else {
      cout << info.size()
           << " info.txt doesn't have enough info, using default data" << endl;
    }
  } else {
    cout << "no info.txt, using default data" << endl;
  }

  cout << "Judging by " << files[0] << " each image should be " << nx << " by "
       << ny << endl;

  // For now assume 8-bit with 1 nm cube voxels
  Voxels v(AlloUInt8Ty, nx, ny, nz, vx, vy, vz, type);

  v.print();

  // Iterate through entire directory
  int slice = 0;
  for (vector<string>::iterator it = files.begin(); it != files.end();
       ++it, ++slice) {
    string &filename = *it;

    if (RGBImage.load(filename)) {
      cout << "loaded " << filename << " (" << slice + 1 << " of "
           << files.size() << ")" << endl;
    } else {
      cout << "Failed to read image from " << filename << endl;
      exit(-4);
    }

    // Verify XY resolution
    if (RGBImage.width() != nx || RGBImage.height() != ny) {
      cout << "Error:  resolution mismatch!" << endl;
      cout << "   " << files[0] << ": " << nx << " by " << ny << endl;
      cout << "   " << filename << ": " << RGBImage.width() << " by "
           << RGBImage.height() << endl;
      exit(-5);
    }

    // Access the read-in image data
    Array &array(RGBImage.array());

    // For now assume 8-bit RGBA
    Image::RGBAPix<unsigned char> pixel;

    // Copy it out pixel-by-pixel:
    for (size_t row = 0; row < array.height(); ++row) {
      for (size_t col = 0; col < array.width(); ++col) {
        array.read(&pixel, col, row);

        // For now we'll take only the red and put it in the single component;
        // that's lame.
        v.elem<char>(0, col, row, slice) = (char)pixel.r;
      }
    }
  }
  // v is ready

  return v;
}

struct AlloApp : App {
  Voxels voxels;
  Isosurface iso;
  AlloApp() {
    Voxels v = voxelsFromDirectory("handouts/examples/fmri_isosurface/images");
    iso.level(2);
    iso.generate(v, 1.0);
  }
};


int main() {
  AlloApp().start();
}
