#include <string>
#include <iostream>
#include "allocore/types/al_Voxels.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"

namespace al{

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