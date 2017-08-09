//#include <cassert>
// #include <algorithm> // min,max
#include "allocore/io/al_MRC.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"
// #include "allocore/graphics/al_Image.hpp"

namespace al {

MRCHeader& MRC::parseMRC(const char * mrcData) {
  MRCHeader& header = *(MRCHeader *)mrcData;

  // check for endian:
  bool swapped = (header.machinestamp[0] == 17);

  if (swapped) {
    printf("swapping byte order...\n");
    swapBytes(&header.nx, 10);
    swapBytes(&header.cella[0], 6);
    swapBytes(&header.mapc, 3);
    swapBytes(&header.dmin, 3);
    swapBytes(&header.ispg, 2);
    swapBytes(&header.origin[0], 3);
    swapBytes(&header.rms, 1);
    swapBytes(&header.nlabl, 1);
  }

  printf("nx: %d ny: %d nz: %d\n", header.nx, header.ny, header.nz);
  printf("mode: ");

  AlloTy ty;

  // set type:
  switch (header.mode) {
    case MRC_IMAGE_SINT8:
      printf("signed int 8\n");
      ty = Array::type<int8_t>();
      break;
    case MRC_IMAGE_SINT16:
      printf("signed int 16\n");
      ty = Array::type<int16_t>();
      break;
    case MRC_IMAGE_FLOAT32:
      printf("float\n");
      ty = Array::type<float>();
      break;
    case MRC_IMAGE_UINT16:
      printf("unsigned int 16\n");
      ty = Array::type<uint16_t>();
      break;
    default:
      printf("MRC mode not supported\n");
      break;
  }

  printf("cell dim: X %f Y %f Z %f\n", header.cella[0], header.cella[1], header.cella[2]);
  printf("axis mapping: X-%d Y-%d Z-%d\n", header.mapc, header.mapr, header.maps);
  printf("density: min %f max %f mean %f rms %f\n", header.dmin, header.dmax, header.dmean, header.rms);
  printf("map: %c%c%c%c\n", header.cmap[0], header.cmap[1], header.cmap[2], header.cmap[3]);
  printf("machine stamp: %d %d %d %d\n", header.machinestamp[0], header.machinestamp[1], header.machinestamp[2], header.machinestamp[3]);
  
  const char * start = mrcData + 1024;

  array().formatAligned(1, ty, header.nx, header.ny, header.nz, 0);
  memcpy(dataPtr(), start, array().size());

  if (swapped) {
    // set type:
    switch (header.mode) {
      case MRC_IMAGE_SINT8:
        swapBytes((int8_t *)dataPtr(), array().cells());
        break;
      case MRC_IMAGE_SINT16:
        swapBytes((int16_t *)dataPtr(), array().cells());
        break;
      case MRC_IMAGE_FLOAT32:
        swapBytes((float *)dataPtr(), array().cells());
        break;
      case MRC_IMAGE_UINT16:
        swapBytes((uint16_t *)dataPtr(), array().cells());
        break;
      default:
        break;
    }
  }

  printf("-- MRC File Read Complete! --\n");

  return header;
}

bool MRC::loadFromMRC(std::string filename) {
  array().zero();

  File data_file(filename, "rb", true);

  printf("[ Reading Data File: %s ]\n", data_file.path().c_str());

  if(!data_file.opened()) {
    AL_WARN("Cannot open MRC file");
    exit(EXIT_FAILURE);
  }

  m_header = parseMRC(data_file.readAll());

  data_file.close();

  return true;
}

bool MRC::writeToMRC(std::string filename) {
  File mrc_file(filename, "wb", true);
  printf("[ Writing MRC File: %s ]\n", mrc_file.path().c_str());

  if(!mrc_file.opened()) {
    AL_WARN("Cannot open MRC file");
    exit(EXIT_FAILURE);
  }
  
  mrc_file.write(&m_header, sizeof(MRCHeader), 1);

  mrc_file.write(dataPtr(), array().size(), 1);
  
  mrc_file.close();
  
  return true;
}

// bool MRC::loadFromBIN(std::string filename) {
//   array().zero();

//   File binary_file(filename, "rb", true);

//   printf("Reading Binary File: %s\n", binary_file.path().c_str());

//   if(!binary_file.opened()) {
//     AL_WARN("Cannot open data file");
//     exit(EXIT_FAILURE);
//   }

//   char validHeader[12];
//   data_file.read(&validHeader, sizeof(char), 12);

//   AlloArrayHeader h2;
//   data_file.read(&h2, sizeof(AlloArrayHeader), 1);
  
//   format(h2);

//   data_file.read(&m_units, sizeof(UnitsTy), 1);
//   data_file.read(&m_voxWidth[0], sizeof(float), 1);
//   data_file.read(&m_voxWidth[1], sizeof(float), 1);
//   data_file.read(&m_voxWidth[2], sizeof(float), 1);
  
//   data_file.read(data.ptr, size(), 1);

//   data_file.close();

//   return true;
// }
  
void MRC::print(FILE * fp) {
  array().print(fp);
  fprintf(fp,"  cell:   %f, %f, %f\n", header().cella[0], header().cella[1], header().cella[2]);
}



// bool MRC::getdir(std::string path, std::vector<std::string> &files) {
//   al::Dir dir;
//   if(dir.open(path)){
//     while(dir.read()){
//       auto& file = dir.entry();
//       if(file.type() == al::FileInfo::REG){
//         auto& name = file.name();
//         if(name != "info.txt" && name != ".DS_Store"){
//           std::cout << name << "\n";
//           files.push_back(path + "/" + name);
//         }
//       }
//     }
//     return true;
//   }
//   return false;
// }
  
    
// bool MRC::parseInfo(std::string dir, std::vector<std::string> &data) {
//   std::string file = dir + "/info.txt";
//   std::cout << file << "\n";
//   std::ifstream infile(file.c_str());
//   if (!infile.good())
//     return false; // exit if file not found

//   std::string strOneLine;

//   while (infile)
//   {
//     getline(infile, strOneLine);
//     if (strOneLine.length() > 0){
//       data.push_back(strOneLine.substr(strOneLine.find(":")+2,strOneLine.length()));
//     }
//   }

//   infile.close();

//   return true;
// }

/*
   sliceassembler

   Voxel data import aka slice assembler aka volume reconstructor 

   by Matt Wright, April 2015
   and Hannah Wolfe, July 2015

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

   - Creates a voxel from the data
*/

// bool MRC::loadFromDirectory(std::string dir) {
  
//   std::vector<std::string> files;
//   std::vector<std::string> info;

//   // Image and Texture handle reading and displaying image files.
//   Image RGBImage; // for reading into
  
//   if (!getdir(dir,files)) {
//     std::cout << "Problem reading directory " << dir << "\n";
//     return false;
//   }

//   if (files.size() == 0) {
//     std::cout << "Read zero files from directory " << dir << "\n";
//     return false;
//   }

//   std::cout << "Judging by " << dir << " there are " << files.size() << " images (or at least files)" << "\n";


//   // Try reading the first one just to get the size
//   if (!RGBImage.load(files[0])) {
//     std::cout << "Couldn't read file " << files[0] << "\n";
//     return false;
//   }

//   int nx = RGBImage.width();
//   int ny = RGBImage.height();
//   int nz = files.size();
//   float vx = 1.;
//   float vy = 1.;
//   float vz = 1.;
//   float type = VOX_NANOMETERS;

//   if (!parseInfo(dir,info)) {
//     if (info.size() == 4) {
//       type = atoi(info[0].c_str());
//       vx = atof(info[1].c_str());
//       vy = atof(info[2].c_str());
//       vz = atof(info[3].c_str());
//       std::cout << "imported values from info.txt: " << type << ", " << vx << ", " << vy << ", " << vz << "\n";
//     } else {
//       std::cout << info.size() << " info.txt doesn't have enough info, using default data" << "\n";
//     }
//   } else {
//     std::cout << "no info.txt, using default data" << "\n";
//   }

//   std::cout << "Judging by " << files[0] << " each image should be " << nx << " by " << ny << "\n";

//   // For now assume 8-bit with 1 nm cube voxels
//   format(1, AlloUInt8Ty, nx, ny, nz);
//   init(vx,vy,vz,type);


//   // Iterate through entire directory
//   int slice = 0;
//   for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it, ++slice) {
//     std::string &filename = *it;

//     if (RGBImage.load(filename)) {
//       std::cout << "loaded " << filename <<
//         " (" << slice+1 << " of " << files.size() << ")" << "\n";
//     } else {
//       std::cout << "Failed to read image from " << filename << "\n";
//       return false;
//     }

//     // Verify XY resolution
//     if (int(RGBImage.width()) != nx || int(RGBImage.height()) != ny) {
//       std::cout << "Error:  resolution mismatch!" << "\n";
//       std::cout << "   " << files[0] << ": " << nx << " by " << ny << "\n";
//       std::cout << "   " << filename << ": " << RGBImage.width() << " by " << RGBImage.height() << "\n";
//       return false;
//     }


//     // Access the read-in image data
//     Array& array(RGBImage.array());
    
//     // For now assume 8-bit RGBA
//     Image::RGBAPix<uint8_t> pixel;

//     // Copy it out pixel-by-pixel:
//     for (size_t row = 0; row < array.height(); ++row) {
//       for (size_t col = 0; col < array.width(); ++col) {
//         array.read(&pixel, col, row);

//         // For now we'll take only the red and put it in the single component; that's lame.
//         elem<char>(0, col, row, slice) = (char) pixel.r;
//       }
//     }
//   }
//   return true;
// }

} // al::
