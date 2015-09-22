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
  printf("mode %d\n", mrcHeader.mode);
  printf("startX %d startY %d startZ %d\n", mrcHeader.startx, mrcHeader.starty, mrcHeader.startz);
  printf("intervals X %d intervals Y %d intervals Z %d\n", mrcHeader.mx, mrcHeader.my, mrcHeader.mz);
  printf("angstroms X %f angstroms Y %f angstroms Z %f\n", mrcHeader.xlen, mrcHeader.ylen, mrcHeader.zlen);
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
      ty = Array::type<float_t>();
      break;
    case MRC_IMAGE_UINT16:
      printf("unsigned int 16\n");
      ty = Array::type<uint16_t>();
      break;
    default:
      printf("MRC mode not supported\n");
      break;
  }

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
        swapBytes((float_t *)data.ptr, cells());
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
  data_file.read(&m_sizex, sizeof(float), 1);
  data_file.read(&m_sizey, sizeof(float), 1);
  data_file.read(&m_sizez, sizeof(float), 1);
  
  data_file.read(data.ptr, size(), 1);

  data_file.close();

  return true;
}

bool Voxels::loadFromMRC(std::string filename) {
  zero();

  File data_file(filename, "rb", true);

  printf("Reading Data File: %s\n", data_file.path().c_str());

  if(!data_file.opened()) {
    AL_WARN("Cannot open MRC file");
    exit(EXIT_FAILURE);
  }

  MRCHeader header = parseMRC(data_file.readAll());

  data_file.close();

  return true;
}

bool Voxels::loadFromMRC(std::string filename, UnitsTy ty, float len) {
  loadFromMRC(filename);

  m_units = ty;
  m_sizex = len;
  m_sizey = len;
  m_sizez = len;

  return true;
}

bool Voxels::loadFromMRC(std::string filename, UnitsTy ty, float lenx, float leny, float lenz) {
  loadFromMRC(filename);

  m_units = ty;
  m_sizex = lenx;
  m_sizey = leny;
  m_sizez = lenz;

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
  voxel_file.write(&m_sizex, sizeof(float), 1);
  voxel_file.write(&m_sizey, sizeof(float), 1);
  voxel_file.write(&m_sizez, sizeof(float), 1);

  voxel_file.write(data.ptr, size(), 1);
  
  voxel_file.close();
  
  return true;
}
  
void Voxels::print(FILE * fp) {
  Array::print(fp);
  fprintf(fp,"  cell:   %s, %s, %s\n", sizexname().c_str(), sizeyname().c_str(), sizezname().c_str());
}

}