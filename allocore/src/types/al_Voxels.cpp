//#include <cassert>
#include <algorithm> // min,max
#include "allocore/types/al_Voxels.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/graphics/al_Image.hpp"

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

bool Voxels::getdir(std::string path, std::vector<std::string> &files) {
	al::Dir dir;
	if(dir.open(path)){
		while(dir.read()){
			auto& file = dir.entry();
			if(file.type() == al::FileInfo::REG){
				auto& name = file.name();
				if(name != "info.txt" && name != ".DS_Store"){
					std::cout << name << "\n";
					files.push_back(path + "/" + name);
				}
			}
		}
		return true;
	}
	return false;
}
  
    
bool Voxels::parseInfo(std::string dir, std::vector<std::string> &data) {
    std::string file = dir + "/info.txt";
    std::cout << file << "\n";
    std::ifstream infile(file.c_str());
    if (!infile.good())
      return false; // exit if file not found

    std::string strOneLine;

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
  
    std::vector<std::string> files;
    std::vector<std::string> info;

    // Image and Texture handle reading and displaying image files.
    Image RGBImage; // for reading into
    
    if (!getdir(dir,files)) {
      std::cout << "Problem reading directory " << dir << "\n";
      return false;
    }

    if (files.size() == 0) {
      std::cout << "Read zero files from directory " << dir << "\n";
      return false;
    }

    std::cout << "Judging by " << dir << " there are " << files.size() << " images (or at least files)" << "\n";


    // Try reading the first one just to get the size
    if (!RGBImage.load(files[0])) {
      std::cout << "Couldn't read file " << files[0] << "\n";
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
      	std::cout << "imported values from info.txt: " << type << ", " << vx << ", " << vy << ", " << vz << "\n";
      } else {
      	std::cout << info.size() << " info.txt doesn't have enough info, using default data" << "\n";
      }
    } else {
      std::cout << "no info.txt, using default data" << "\n";
    }

    std::cout << "Judging by " << files[0] << " each image should be " << nx << " by " << ny << "\n";

    // For now assume 8-bit with 1 nm cube voxels
    format(1, AlloUInt8Ty, nx, ny, nz);
    init(vx,vy,vz,type);


    // Iterate through entire directory
    int slice = 0;
    for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it, ++slice) {
      std::string &filename = *it;

      if (RGBImage.load(filename)) {
      	std::cout << "loaded " << filename <<
      	  " (" << slice+1 << " of " << files.size() << ")" << "\n";
      } else {
      	std::cout << "Failed to read image from " << filename << "\n";
      	return false;
      }

      // Verify XY resolution
      if (int(RGBImage.width()) != nx || int(RGBImage.height()) != ny) {
      	std::cout << "Error:  resolution mismatch!" << "\n";
      	std::cout << "   " << files[0] << ": " << nx << " by " << ny << "\n";
      	std::cout << "   " << filename << ": " << RGBImage.width() << " by " << RGBImage.height() << "\n";
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

/*  //BACK-UP
bool Voxels::linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection)
{
   Vec3f P10 = P1 - P0;
   Vec3f P20 = planeCenter - P0;
   float nDot10 = planeNormal.dot(P10);
   float nDot20 = planeNormal.dot(P20);

   if (nDot10 == 0){
     return false;
   }

   float u = nDot20/nDot10;

   if (u > 1.0 or u < 0.0) {
     return false;
   }

   *intersection = u*P0 + (1-u)*P1;

   return true;
}

//back 2
bool Voxels::linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection)
{
   Vec3f P10 = P1 - P0;
   Vec3f P20 = planeCenter - P0;
   float nDot10 = planeNormal.dot(P10);
   float nDot20 = planeNormal.dot(P20);
   Vec3f w = P0 -planeCenter;
   float fac = -planeNormal.dot(w)/nDot10;
   Vec3f t = P10*fac;

   if (nDot10 == 0){
     return false;
   }

   float u = nDot20/nDot10;

   if (u > 1.0 or u < 0.0) {
     return false;
   }

  *intersection = P0 +t;  //testing a new intersect calculation

   return true;
}
*/

bool Voxels::linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection)
{
   Vec3f P10 = P1 - P0;
   Vec3f P20 = planeCenter - P0;
   Vec3f P02 = P0 - planeCenter;
   float nDot10 = planeNormal.dot(P10);
   float nDot20 = planeNormal.dot(P20);

   if (nDot10 == 0){
     return false;
   }

   float u = nDot20/nDot10;

   if (u > 1.0 || u < 0.0) {
     return false;
   }

  *intersection = P0 +P10*u;  //testing a new intersect calculation

   return true;
}



std::vector<Vec3f> Voxels::linspace( Vec3f a, Vec3f b, int n) {
  std::vector<Vec3f> arr;
  Vec3f ba = b-a;
  Vec3f step = Vec3f(ba.x/(n-1.0),ba.y/(n-1.0),ba.z/(n-1.0));
//  std::cout << ba.x << " " << ba.y << " " << ba.z << "\n";
//  std::cout << step.x << " " << step.y << " " << step.z << "\n";

  for (int i = 0; i < n; i++) {
    arr.push_back(a);
    a += step;
  }
  return arr;
}

Vec3f Voxels::point2Dto3D(Vec3f Q, Vec3f H, Vec3f K, float u, float v){
  return Vec3f(Q.x+u*H.x+v*K.x,Q.y+u*H.y+v*K.y,Q.z+u*H.z+v*K.z);
}

bool Voxels::parallelLinespace(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f p3, std::vector<Vec3f> &list, std::vector<Vec3f> &list2, float aDirection, float oDirection, std::vector<Vec3f> &points){
  Vec3f a = p0-p1;
  Vec3f b = p2-p3;
  float t = a.dot(b)/(a.mag()*b.mag());
  t = round(t*10000)/10000;
  int n = aDirection;
  if ((p0-p1).mag()/(p0-p2).mag() == aDirection/oDirection){ 
    n = oDirection; 
  }
  std::cout.flush();
  std::cout << "t = " << t << "\n";
  if (t == -1.0){
    list = linspace(p0,p1,n);
    list2 = linspace(p3,p2,n);
    points.push_back(p0);
    points.push_back(p1);
    points.push_back(p3);
    points.push_back(p2);
    return true;
  } else if (t == 1.0){
    list = linspace(p0,p1,n);
    list2 = linspace(p2,p3,n);
    points.push_back(p0);
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    return true;
  } 
  return false;
}

//does this needs to be in voxels?  currently it isn't
Array Voxels::slice(Vec3f planeCenter, Vec3f planeNormal, std::vector<Vec3f> &finalPointList){
 //assume point and vector are given in cell*width, cell*height, cell*depth coordinate system
 //point and vector define a plane
 // nice page on cube plane intersection
 // http://cococubed.asu.edu/code_pages/raybox.shtml
 // calculate Maxs
  float xMax =  width()* m_voxWidth[0];
  float yMax =  height()* m_voxWidth[1];
  float zMax =  depth()* m_voxWidth[2];

  std::cout << "values " << xMax << " " << yMax << " " << zMax << "\n";
//
//calculate intersections 
  std::vector<Vec3f> P;
  Vec3f intersection;
  if (linePlaneIntersection(Vec3f(0,0,0),Vec3f(0,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,0),Vec3f(0,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,yMax,0),Vec3f(xMax,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,0),Vec3f(xMax,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,0,0),Vec3f(xMax,0,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,0),Vec3f(xMax,yMax,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,zMax),Vec3f(xMax,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,zMax),Vec3f(0,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,0,0),Vec3f(0,yMax,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,zMax),Vec3f(0,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,zMax),Vec3f(xMax,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,0),Vec3f(xMax,yMax,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);

  Array result = Array();// XXX
  
  if (P.size() > 1){
    Vec2f *p2D = new Vec2f[P.size()];
    p2D[0] = Vec2f(0,0);
    float x = sqrt(pow(P[1].x-P[0].x,2.0)+pow(P[1].y-P[0].y,2.0)+pow(P[1].z-P[0].z,2.0));
    std::cout << x << "\n";
    p2D[1] = Vec2f(x,0);
    if(P.size() == 2){
      //super easy, it's just a line :)
      x = ceil(x);
      result.format(1, type(), x);
      std::vector<Vec3f> space = linspace(P[0], P[1], x);
      for (unsigned j = 0; j < space.size(); j++){
        Vec3f point = space[j];
        float temp[1] = {0};
        if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  width()* m_voxWidth[2] && point.y <=  height()* m_voxWidth[1] && point.z <=  depth()* m_voxWidth[2]){
          Vec3f p = Vec3f(point.x/ m_voxWidth[0],point.y/ m_voxWidth[1],point.z/ m_voxWidth[2]);
           read_interp(temp, p);
        }
        result.write(temp,j);
      }
    } else {
      //Oh noes!  It's more than a line, this is a little more complex
      float minA2D = 0, minO2D = 0, maxA2D = 0, maxO2D = 0;
      //what are all these points in 2d on a plane
      //http://stackoverflow.com/questions/10702099/computing-two-vectors-that-are-perpendicular-to-third-vector-in-3d
      Vec3f y_axis = planeNormal.cross((P[0]-planeCenter).normalize());
      Vec3f z_axis = planeNormal.cross(y_axis);
      //http://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
      for (unsigned i = 0; i < P.size(); i++) {
	Vec3f pi = P[i]-planeCenter;
        float t_1 = y_axis.dot(pi);
        float t_2 = z_axis.dot(pi);
        minA2D = std::min(minA2D, t_1);
        maxA2D = std::max(maxA2D, t_1);
        minO2D = std::min(minO2D, t_2);
        maxO2D = std::max(maxO2D, t_2);
        p2D[i] = Vec2f(t_1, t_2);
      }
      int aDirection = ceil(maxA2D - minA2D);
      int oDirection = ceil(maxO2D - minO2D);
      //http://math.stackexchange.com/questions/525829/how-to-find-the-3d-coordinate-of-a-2d-point-on-a-known-plane
      result.format(1, type(), aDirection, oDirection);
      Vec3f p0 = point2Dto3D(planeCenter,y_axis,z_axis,minA2D,minO2D);
      Vec3f p1 = point2Dto3D(planeCenter,y_axis,z_axis,minA2D,maxO2D);
      Vec3f p2 = point2Dto3D(planeCenter,y_axis,z_axis,maxA2D,maxO2D);
      Vec3f p3 = point2Dto3D(planeCenter,y_axis,z_axis,maxA2D,minO2D);
      std::cout << "p0 :" << p0.x << " " << p0.y << " " << p0.z << "\n";
      std::cout << "p1 :" << p1.x << " " << p1.y << " " << p1.z << "\n";
      std::cout << "p2 :" << p2.x << " " << p2.y << " " << p2.z << "\n";
      std::cout << "p3 :" << p3.x << " " << p3.y << " " << p3.z << "\n";
      //Check to see if two lines intersect
      std::vector<Vec3f> list;
      std::vector<Vec3f> list2;
      if (!parallelLinespace(p0, p1, p2, p3, list, list2, maxA2D-minA2D, maxO2D-minO2D, finalPointList)){
        if (!parallelLinespace(p0, p2, p1, p3, list, list2, maxA2D-minA2D, maxO2D-minO2D, finalPointList)){
          parallelLinespace(p0, p3, p1, p2, list, list2, maxA2D-minA2D, maxO2D-minO2D, finalPointList);
        }
      }
      std::cout << "finalPointList Length:" << finalPointList.size() << "\n";
      //now lets fill the results
      for (unsigned i = 0; i < list.size(); i++){
        std::vector<Vec3f> space = linspace(list[i], list2[i], oDirection);  //XXX should this be oDirection or aDirection, please check!
        for (unsigned j = 0; j < space.size(); j++){
	  Vec3f point = space[j];
   	  float temp[1] = {0};
	  if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  width()* m_voxWidth[2] && point.y <=  height()* m_voxWidth[1] && point.z <=  depth()* m_voxWidth[2]){
 	    Vec3f p = Vec3f(point.x/ m_voxWidth[0],point.y/ m_voxWidth[1],point.z/ m_voxWidth[2]);
  	     read_interp(temp, p);
	  }
	  result.write(temp,i,j);
	}
      }
    }
   delete [] p2D; 
  }
  else if (P.size() == 1) {
    //Intersects at one point, this is super easy!
    //calculate point and return array with single point
    result.format(1, type(),1);
    Vec3f point = P[0];
    float temp[1] = {0};
    if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  width()* m_voxWidth[2] && point.y <=  height()* m_voxWidth[1] && point.z <=  depth()* m_voxWidth[2]){
      Vec3f p = Vec3f(point.x/ m_voxWidth[0],point.y/ m_voxWidth[1],point.z/ m_voxWidth[2]);
       read_interp(temp, p);
    }
    result.write(temp,0);
  }
//  for (int i = 0; i < finalPointList.size(); i++){
//    finalPointList[i] = finalPointList[i]/Vec3f(xMax,yMax,zMax);
//  }
//  std::cout << "wtf" << "\n";
  return result;
}


} // al::
