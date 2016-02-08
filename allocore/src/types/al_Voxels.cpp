#include <string>
#include <iostream>
#include "allocore/types/al_Voxels.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"

namespace al {

MRCHeader& Voxels::parseMRC(const char * mrcData) {
  MRCHeader& mrcHeader = *(MRCHeader *)mrcData;

  // check for byte swap:
  bool swapped =
    (mrcHeader.nx <= 0 || mrcHeader.ny <= 0 || mrcHeader.nz <= 0 ||
    (mrcHeader.nx > 65535 && mrcHeader.ny > 65535 && mrcHeader.nz > 65535) ||
    mrcHeader.mapx < 0 || mrcHeader.mapx > 4 ||
    mrcHeader.mapy < 0 || mrcHeader.mapy > 4 ||
    mrcHeader.mapz < 0 || mrcHeader.mapz > 4);

  // ugh.
  if (swapped) {
    printf("swapping byte order...\n");
    swapBytes(&mrcHeader.nx, 10);
    swapBytes(&mrcHeader.xlen, 6);
    swapBytes(&mrcHeader.mapx, 3);
    swapBytes(&mrcHeader.amin, 3);
    swapBytes(&mrcHeader.ispg, 2);
    swapBytes(&mrcHeader.next, 1);
    swapBytes(&mrcHeader.creatid, 1);
    swapBytes(&mrcHeader.nint, 4);
    swapBytes(&mrcHeader.min2, 4);
    swapBytes(&mrcHeader.imodStamp, 2);
    swapBytes(&mrcHeader.idtype, 6);
    swapBytes(&mrcHeader.tiltangles[0], 6);
    swapBytes(&mrcHeader.origin[0], 3);
    swapBytes(&mrcHeader.rms, 1);
    swapBytes(&mrcHeader.nlabl, 1);
  }

  printf("NX %d NY %d NZ %d\n", mrcHeader.nx, mrcHeader.ny, mrcHeader.nz);
  printf("mode ");

  AlloTy ty;

  // set type:
  switch (mrcHeader.mode) {
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

  printf("cell dimensions X %f Y %f Z %f\n", mrcHeader.xlen, mrcHeader.ylen, mrcHeader.zlen);
  printf("axis X %d axis Y %d axis Z %d\n", mrcHeader.mapx, mrcHeader.mapy, mrcHeader.mapz);
  printf("density min %f max %f mean %f\n", mrcHeader.amin, mrcHeader.amax, mrcHeader.amean);
  printf("origin %f %f %f\n", mrcHeader.origin[0], mrcHeader.origin[1], mrcHeader.origin[2]);
  printf("map %s\n", mrcHeader.cmap);
  printf("machine stamp %s\n", mrcHeader.machinestamp);
  printf("rms %f\n", mrcHeader.rms);
  printf("labels %d\n", mrcHeader.nlabl);
  for (int i=0; i<mrcHeader.nlabl; i++) {
    //printf("\t%02d: %s\n", i, mrcHeader.labels[i]);
  }

  const char * start = mrcData + 1024;

  formatAligned(1, ty, mrcHeader.nx, mrcHeader.ny, mrcHeader.nz, 0);
  memcpy(data.ptr, start, size());

  if (swapped) {
    // set type:
    switch (mrcHeader.mode) {
      case MRC_IMAGE_SINT8:
        swapBytes((int8_t *)data.ptr, cells());
        break;
      case MRC_IMAGE_SINT16:
        swapBytes((int16_t *)data.ptr, cells());
        break;
      case MRC_IMAGE_FLOAT32:
        swapBytes((float *)data.ptr, cells());
        break;
      case MRC_IMAGE_UINT16:
        swapBytes((uint16_t *)data.ptr, cells());
        break;
      default:
        break;
    }
  }

  return mrcHeader;
}

bool Voxels::loadFromMRC(std::string filename, bool update) {
  zero();

  File data_file(filename, "rb", true);

  printf("Reading Data File: %s\n", data_file.path().c_str());

  if(!data_file.opened()) {
    AL_WARN("Cannot open MRC file");
    exit(EXIT_FAILURE);
  }

  MRCHeader header = parseMRC(data_file.readAll());

  data_file.close();

  if (update) {
    // convert into angstrom
    header.xlen = m_voxWidth[0] * powf(10.0, m_units + 10);
    header.ylen = m_voxWidth[1] * powf(10.0, m_units + 10);
    header.zlen = m_voxWidth[2] * powf(10.0, m_units + 10);

    writeToMRC(filename + "_new", header);
  } else {
    m_units = VOX_NANOMETERS; // default to nanometers
    m_voxWidth[0] = header.xlen * 0.1f;
    m_voxWidth[1] = header.ylen * 0.1f;
    m_voxWidth[2] = header.zlen * 0.1f;
  }

  m_min = header.amin;
  m_max = header.amax;
  m_mean = header.amean;
  m_rms = header.rms;

  return true;
}

bool Voxels::loadFromMRC(std::string filename, UnitsTy ty, float voxWidth) {
  m_units = ty;
  m_voxWidth[0] = voxWidth;
  m_voxWidth[1] = voxWidth;
  m_voxWidth[2] = voxWidth;

  loadFromMRC(filename, true);

  return true;
}

bool Voxels::loadFromMRC(std::string filename, UnitsTy ty, float voxWidthX, float voxWidthY, float voxWidthZ) {
  m_units = ty;
  m_voxWidth[0] = voxWidthX;
  m_voxWidth[1] = voxWidthY;
  m_voxWidth[2] = voxWidthZ;

  loadFromMRC(filename, true);
  
  return true;
}

bool Voxels::writeToMRC(std::string filename, MRCHeader& header) {
  File mrc_file(filename, "wb", true);
  printf("Writing MRC File: %s\n", mrc_file.path().c_str());

  if(!mrc_file.opened()) {
    AL_WARN("Cannot open MRC file");
    exit(EXIT_FAILURE);
  }
  
  mrc_file.write(&header, sizeof(MRCHeader), 1);

  mrc_file.write(data.ptr, size(), 1);
  
  mrc_file.close();
  
  return true;
}

bool Voxels::loadFromFile(std::string filename) {
  zero();

  File data_file(filename, "rb", true);

  printf("Reading Data File: %s\n", data_file.path().c_str());

  if(!data_file.opened()) {
    AL_WARN("Cannot open data file");
    exit(EXIT_FAILURE);
  }
  char validHeader[12];
  data_file.read(&validHeader, sizeof(char), 12);

  AlloArrayHeader h2;
  data_file.read(&h2, sizeof(AlloArrayHeader), 1);
  
  format(h2);

  data_file.read(&m_units, sizeof(UnitsTy), 1);
  data_file.read(&m_voxWidth[0], sizeof(float), 1);
  data_file.read(&m_voxWidth[1], sizeof(float), 1);
  data_file.read(&m_voxWidth[2], sizeof(float), 1);
  
  data_file.read(data.ptr, size(), 1);

  data_file.close();

  return true;
}

bool Voxels::writeToFile(std::string filename) {
  File voxel_file(filename, "wb", true);
  printf("Writing Voxel File: %s\n", voxel_file.path().c_str());

  if(!voxel_file.opened()) {
    AL_WARN("Cannot open voxel file");
    exit(EXIT_FAILURE);
  }

  char validHeader[12] = "Allo Voxels";

  voxel_file.write(validHeader, sizeof(char), 12);
  
  voxel_file.write(&header, sizeof(AlloArrayHeader), 1);

  voxel_file.write(&m_units, sizeof(UnitsTy), 1);
  voxel_file.write(&m_voxWidth[0], sizeof(float), 1);
  voxel_file.write(&m_voxWidth[1], sizeof(float), 1);
  voxel_file.write(&m_voxWidth[2], sizeof(float), 1);

  voxel_file.write(data.ptr, size(), 1);
  
  voxel_file.close();
  
  return true;
}
  
void Voxels::print(FILE * fp) {
  Array::print(fp);
  fprintf(fp,"  cell:   %s, %s, %s\n", printVoxWidth(0).c_str(), printVoxWidth(1).c_str(), printVoxWidth(2).c_str());
}

bool Voxels::getdir(std::string dir, vector<std::string> &files) {
    DIR *dp;
    struct dirent * dirp;
    if((dp = opendir(dir.c_str())) == NULL)	{
      //cout << "Error(" << errno << ") opening " << dir << endl;
      return false;
    }
    while((dirp = readdir(dp)) != NULL) {
      char *name = dirp->d_name;
      if (strcmp(name, ".") && strcmp(name, "..") && strcmp(name, "info.txt") && strcmp(name, ".DS_Store")) {
      	cout << name << endl;
      	files.push_back(dir + "/" + string(name));
      }
    }
    closedir(dp);
    return true;
  }
  
    
bool Voxels::parseInfo(std::string dir, vector<std::string> &data) {
    string file = dir + "/info.txt";
    cout << file << endl;
    ifstream infile(file.c_str());
    if (!infile.good())
      return false; // exit if file not found

    string strOneLine;

    while (infile)
    {
      getline(infile, strOneLine);
      if (strOneLine.length() > 0){
      	data.push_back(strOneLine.substr(strOneLine.find(":")+2,strOneLine.length()));
      }
    }

    infile.close();

    return true;
  }

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

bool Voxels::loadFromDirectory(std::string dir) {
  
    vector<string> files;
    vector<string> info;

    // Image and Texture handle reading and displaying image files.
    Image RGBImage; // for reading into
    
    if (!getdir(dir,files)) {
      cout << "Problem reading directory " << dir << endl;
      return false;
    }

    if (files.size() == 0) {
      cout << "Read zero files from directory " << dir << endl;
      return false;
    }

    cout << "Judging by " << dir << " there are " << files.size() << " images (or at least files)" << endl;


    // Try reading the first one just to get the size
    if (!RGBImage.load(files[0])) {
      cout << "Couldn't read file " << files[0] << endl;
      return false;
    }

    int nx = RGBImage.width();
    int ny = RGBImage.height();
    int nz = files.size();
    float vx = 1.;
    float vy = 1.;
    float vz = 1.;
    float type = VOX_NANOMETERS;

    if (!parseInfo(dir,info)) {
      if (info.size() == 4) {
      	type = atoi(info[0].c_str());
      	vx = atof(info[1].c_str());
      	vy = atof(info[2].c_str());
      	vz = atof(info[3].c_str());
      	cout << "imported values from info.txt: " << type << ", " << vx << ", " << vy << ", " << vz << endl;
      } else {
      	cout << info.size() << " info.txt doesn't have enough info, using default data" << endl;
      }
    } else {
      cout << "no info.txt, using default data" << endl;
    }

    cout << "Judging by " << files[0] << " each image should be " << nx << " by " << ny << endl;

    // For now assume 8-bit with 1 nm cube voxels
    format(1, AlloUInt8Ty, nx, ny, nz);
    init(vx,vy,vz,type);


    // Iterate through entire directory
    int slice = 0;
    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it, ++slice) {
      string &filename = *it;

      if (RGBImage.load(filename)) {
      	cout << "loaded " << filename << 
      	  " (" << slice+1 << " of " << files.size() << ")" << endl;
      } else {
      	cout << "Failed to read image from " << filename << endl;
      	return false;
      }

      // Verify XY resolution
      if (int(RGBImage.width()) != nx || int(RGBImage.height()) != ny) {
      	cout << "Error:  resolution mismatch!" << endl;
      	cout << "   " << files[0] << ": " << nx << " by " << ny << endl;
      	cout << "   " << filename << ": " << RGBImage.width() << " by " << RGBImage.height() << endl;
      	return false;
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
      	  elem<char>(0, col, row, slice) = (char) pixel.r;
      	}
      }
    }
    return true;
  }

 
}
