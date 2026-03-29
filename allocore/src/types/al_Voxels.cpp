//#include <cassert>
#include <algorithm> // min,max
#include <cmath> // ceil, pow, sqrt
#include <cstring> // memcpy
#include <fstream>
#include <iostream>
#include <sstream>
#include "allocore/types/al_Voxels.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/types/al_Conversion.hpp"

namespace al {

Voxels::Voxels()
:	Array()
{
	init(1,1,1, VOX_METERS);
}

Voxels::Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez, UnitsTy units)
:	Array(1, ty, dimx, dimy, dimz)
{
	init(sizex, sizey, sizez, units);
}

Voxels::Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float voxelsize, UnitsTy units)
:	Voxels(ty, dimx,dimy,dimz, voxelsize,voxelsize,voxelsize, units)
{}

void Voxels::init(float voxWidthX, float voxWidthY, float voxWidthZ, UnitsTy units){
	mVoxWidth[0] = voxWidthX;
	mVoxWidth[1] = voxWidthY;
	mVoxWidth[2] = voxWidthZ;
	mUnits = units;
}

float Voxels::getVoxWidth(unsigned int axis) const { return mVoxWidth[axis]; }
void Voxels::setVoxWidth(unsigned int axis, float voxWidth){ mVoxWidth[axis] = voxWidth; }

MRCHeader& Voxels::parseMRC(const char * mrcData){
	auto& hdr = *(MRCHeader *)mrcData;

	// check for byte swap:
	bool swapped =
		(hdr.nx <= 0 || hdr.ny <= 0 || hdr.nz <= 0 ||
		(hdr.nx > 65535 && hdr.ny > 65535 && hdr.nz > 65535) ||
		hdr.mapx < 0 || hdr.mapx > 4 ||
		hdr.mapy < 0 || hdr.mapy > 4 ||
		hdr.mapz < 0 || hdr.mapz > 4)
	;

	// ugh.
	if(swapped){
		printf("swapping byte order...\n");
		swapBytes(&hdr.nx, 10);
		swapBytes(&hdr.xlen, 6);
		swapBytes(&hdr.mapx, 3);
		swapBytes(&hdr.amin, 3);
		swapBytes(&hdr.ispg, 2);
		swapBytes(&hdr.next, 1);
		swapBytes(&hdr.creatid, 1);
		swapBytes(&hdr.nint, 4);
		swapBytes(&hdr.min2, 4);
		swapBytes(&hdr.imodStamp, 2);
		swapBytes(&hdr.idtype, 6);
		swapBytes(&hdr.tiltangles[0], 6);
		swapBytes(&hdr.origin[0], 3);
		swapBytes(&hdr.rms, 1);
		swapBytes(&hdr.nlabl, 1);
	}

	printf("NX %d NY %d NZ %d\n", hdr.nx, hdr.ny, hdr.nz);
	printf("mode ");

	AlloTy ty;

	// set type:
	switch(hdr.mode){
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

	printf("cell dimensions X %f Y %f Z %f\n", hdr.xlen, hdr.ylen, hdr.zlen);
	printf("axis X %d axis Y %d axis Z %d\n", hdr.mapx, hdr.mapy, hdr.mapz);
	printf("density min %f max %f mean %f\n", hdr.amin, hdr.amax, hdr.amean);
	printf("origin %f %f %f\n", hdr.origin[0], hdr.origin[1], hdr.origin[2]);
	printf("map %s\n", hdr.cmap);
	printf("machine stamp %s\n", hdr.machinestamp);
	printf("rms %f\n", hdr.rms);
	printf("labels %d\n", hdr.nlabl);
	for (int i=0; i<hdr.nlabl; i++) {
		//printf("\t%02d: %s\n", i, hdr.labels[i]);
	}

	const char * start = mrcData + 1024;

	formatAligned(1, ty, hdr.nx, hdr.ny, hdr.nz, 0);
	memcpy(data.ptr, start, size());

	if(swapped){
		// set type:
		switch (hdr.mode) {
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

	return hdr;
}

bool Voxels::loadFromMRC(std::string filename, bool update){
	zero();

	File f(filename, "rb", true);

	printf("Reading Data File: %s\n", f.path().c_str());

	if(!f.opened()){
		AL_WARN("Cannot open MRC file");
		return false;
	}

	auto hdr = parseMRC(f.readAll());

	if(update){
		// convert into angstrom
		hdr.xlen = mVoxWidth[0] * powf(10.0, mUnits + 10);
		hdr.ylen = mVoxWidth[1] * powf(10.0, mUnits + 10);
		hdr.zlen = mVoxWidth[2] * powf(10.0, mUnits + 10);
		writeToMRC(filename + "_new", hdr);
	} else {
		mUnits = VOX_NANOMETERS; // default to nanometers
		mVoxWidth[0] = hdr.xlen * 0.1f;
		mVoxWidth[1] = hdr.ylen * 0.1f;
		mVoxWidth[2] = hdr.zlen * 0.1f;
	}

	mMin = hdr.amin;
	mMax = hdr.amax;
	mMean = hdr.amean;
	mRMS = hdr.rms;

	return true;
}

bool Voxels::loadFromMRC(std::string filename, UnitsTy ty, float voxWidth){
	return loadFromMRC(filename, ty, voxWidth,voxWidth,voxWidth);
}

bool Voxels::loadFromMRC(std::string filename, UnitsTy ty, float voxWidthX, float voxWidthY, float voxWidthZ){
	mUnits = ty;
	mVoxWidth[0] = voxWidthX;
	mVoxWidth[1] = voxWidthY;
	mVoxWidth[2] = voxWidthZ;
	return loadFromMRC(filename, true);
}

bool Voxels::writeToMRC(std::string filename, MRCHeader& header){
	File f(filename, "wb", true);
	printf("Writing MRC File: %s\n", f.path().c_str());

	if(!f.opened()) {
		AL_WARN("Cannot open MRC file");
		return false;
	}

	f.write(header);
	f.write(data.ptr, size(), 1);

	return true;
}

bool Voxels::loadFromFile(std::string filename){
	zero();

	File f(filename, "rb", true);

	printf("Reading Data File: %s\n", f.path().c_str());

	if(!f.opened()) {
		AL_WARN("Cannot open data file");
		return false;
	}
	char validHeader[12];
	f.read(&validHeader, sizeof(char), 12);

	AlloArrayHeader h2;
	f.read(h2);
	format(h2);

	f.read(mUnits);
	f.read(mVoxWidth[0]);
	f.read(mVoxWidth[1]);
	f.read(mVoxWidth[2]);
	f.read(data.ptr, size(), 1);

	return true;
}

bool Voxels::writeToFile(std::string filename) {
	File f(filename, "wb", true);
	printf("Writing Voxel File: %s\n", f.path().c_str());

	if(!f.opened()) {
		AL_WARN("Cannot open voxel file");
		return false;
	}

	char validHeader[12] = "Allo Voxels";

	f.write(validHeader, sizeof(char), 12);
	f.write(header);
	f.write(mUnits);
	f.write(mVoxWidth[0]);
	f.write(mVoxWidth[1]);
	f.write(mVoxWidth[2]);
	f.write(data.ptr, size(), 1);

	return true;
}

void Voxels::print(FILE * fp) const {
	Array::print(fp);
	fprintf(fp,"  cell:   %s, %s, %s\n", printVoxWidth(0).c_str(), printVoxWidth(1).c_str(), printVoxWidth(2).c_str());
}

std::string Voxels::printVoxWidth(unsigned int axis) const {
	std::ostringstream ss;
	ss << mVoxWidth[axis] << " " << printUnits(mUnits);
	return ss.str();
}

std::string Voxels::printUnits(UnitsTy t) const {
	if(t == VOX_ANGSTROMS){
		return "angstroms";
	} else if(t == VOX_NANOMETERS){
		return "nm";
	} else if(t == VOX_MICROMETERS){
		return "µm";
	} else if(t == VOX_MILLIMETERS){
		return "mm";
	} else if(t == VOX_CENTIMETERS){
		return "cm";
	} else {
		std::ostringstream ss;
		ss << "(m*10^" << t << ")";
		return ss.str();
	}
}

std::string Voxels::printUnits() const {
	return printUnits(mUnits);
}

bool Voxels::getdir(std::string path, std::vector<std::string> &files) {
	al::Dir dir;
	if(dir.open(path)){
		while(dir.read()){
			auto& file = dir.entry();
			if(file.type() == al::FileInfo::REG){
				auto& name = file.name();
				if(name != "info.txt" && name != ".DS_Store"){
					//printf("%s\n", name.c_str());
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
	if(!infile.good())
		return false;

	std::string strOneLine;

	while(infile){
		getline(infile, strOneLine);
		if(strOneLine.length() > 0){
			data.push_back(strOneLine.substr(strOneLine.find(":")+2,strOneLine.length()));
		}
	}

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
	Image img; // for reading into

	if(!getdir(dir,files)){
		std::cout << "Problem reading directory " << dir << "\n";
		return false;
	}

	if(files.size() == 0){
		std::cout << "Read zero files from directory " << dir << "\n";
		return false;
	}

	std::cout << "Judging by " << dir << " there are " << files.size() << " images (or at least files)" << "\n";


	// Try reading the first one just to get the size
	if(!img.load(files[0])){
		std::cout << "Couldn't read file " << files[0] << "\n";
		return false;
	}

	int nx = img.width();
	int ny = img.height();
	int nz = files.size();
	float vx = 1.;
	float vy = 1.;
	float vz = 1.;
	float type = VOX_NANOMETERS;

	if(!parseInfo(dir,info)){
		if(info.size() == 4){
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

		if(img.load(filename)){
			std::cout << "loaded " << filename <<
			" (" << slice+1 << " of " << files.size() << ")" << "\n";
		} else {
			std::cout << "Failed to read image from " << filename << "\n";
			return false;
		}

		// Verify XY resolution
		if(int(img.width()) != nx || int(img.height()) != ny){
			std::cout << "Error:  resolution mismatch!" << "\n";
			std::cout << "   " << files[0] << ": " << nx << " by " << ny << "\n";
			std::cout << "   " << filename << ": " << img.width() << " by " << img.height() << "\n";
			return false;
		}


		// Access the read-in image data
		auto& array = img.array();

		// For now assume 8-bit RGBA
		Image::RGBAPix<uint8_t> pixel;

		// Copy it out pixel-by-pixel:
		for(size_t row = 0; row < array.height(); ++row){
			for(size_t col = 0; col < array.width(); ++col){
				array.read(&pixel, col, row);

				// For now we'll take only the red and put it in the single component; that's lame.
				elem<char>(0, col, row, slice) = (char) pixel.r;
			}
		}
	}
	return true;
}

bool Voxels::linePlaneIntersection(const Vec3f& P0, const Vec3f& P1, const Vec3f& planeCenter, const Vec3f& planeNormal, Vec3f * intersection){
	auto P10 = P1 - P0;
	auto P20 = planeCenter - P0;
	auto P02 = P0 - planeCenter;
	auto nDot10 = planeNormal.dot(P10);
	auto nDot20 = planeNormal.dot(P20);

	if(nDot10 == 0)
		return false;

	auto u = nDot20/nDot10;

	if(u > 1.0 || u < 0.0)
		return false;

	*intersection = P0 + P10*u;  //testing a new intersect calculation

	return true;
}

std::vector<Vec3f> Voxels::linspace(Vec3f a, Vec3f b, int n){
	std::vector<Vec3f> arr;
	auto ba = b-a;
	auto step = Vec3f(ba.x/(n-1.0),ba.y/(n-1.0),ba.z/(n-1.0));
	//  std::cout << ba.x << " " << ba.y << " " << ba.z << "\n";
	//  std::cout << step.x << " " << step.y << " " << step.z << "\n";

	for(int i = 0; i < n; i++){
		arr.push_back(a);
		a += step;
	}
	return arr;
}

Vec3f Voxels::point2Dto3D(Vec3f Q, Vec3f H, Vec3f K, float u, float v){
	return Vec3f(Q.x+u*H.x+v*K.x,Q.y+u*H.y+v*K.y,Q.z+u*H.z+v*K.z);
}

bool Voxels::parallelLinespace(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f p3, std::vector<Vec3f>& list, std::vector<Vec3f>& list2, float aDirection, float oDirection, std::vector<Vec3f>& points){
	auto a = p0-p1;
	auto b = p2-p3;
	auto t = a.dot(b)/(a.mag()*b.mag());
	t = round(t*10000)/10000;
	int n = aDirection;
	if((p0-p1).mag()/(p0-p2).mag() == aDirection/oDirection){
		n = oDirection;
	}
	std::cout.flush();
	std::cout << "t = " << t << "\n";
	if(t == -1.0){
		list = linspace(p0,p1,n);
		list2 = linspace(p3,p2,n);
		points.push_back(p0);
		points.push_back(p1);
		points.push_back(p3);
		points.push_back(p2);
		return true;
	} else if(t == 1.0){
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
Array Voxels::slice(Vec3f planeCenter, Vec3f planeNormal, std::vector<Vec3f>& finalPointList){
	//assume point and vector are given in cell*width, cell*height, cell*depth coordinate system
	//point and vector define a plane
	// nice page on cube plane intersection
	// http://cococubed.asu.edu/code_pages/raybox.shtml
	// calculate Maxs
	auto W =  width() * mVoxWidth[0];
	auto H = height() * mVoxWidth[1];
	auto D =  depth() * mVoxWidth[2];

	std::cout << "values " << W << " " << H << " " << D << "\n";
	//
	//calculate intersections
	std::vector<Vec3f> P;
	Vec3f intersection;
	#define CHECK_INTERSECTION(x1,y1,z1, x2,y2,z2)\
		if(linePlaneIntersection(Vec3f(x1,y1,z1),Vec3f(x2,y2,z2), planeCenter, planeNormal, &intersection))\
			P.push_back(intersection)

	CHECK_INTERSECTION(0,0,0, 0,0,D);
	CHECK_INTERSECTION(0,H,0, 0,H,D);
	CHECK_INTERSECTION(W,H,0, W,H,D);
	CHECK_INTERSECTION(W,0,0, W,0,D);
	CHECK_INTERSECTION(0,0,0, W,0,0);
	CHECK_INTERSECTION(0,H,0, W,H,0);
	CHECK_INTERSECTION(0,H,D, W,H,D);
	CHECK_INTERSECTION(W,0,D, 0,0,D);
	CHECK_INTERSECTION(0,0,0, 0,H,0);
	CHECK_INTERSECTION(0,H,D, 0,0,D);
	CHECK_INTERSECTION(W,0,D, W,H,D);
	CHECK_INTERSECTION(W,0,0, W,H,0);

	Array result = Array();// XXX

	if(P.size() > 1){
		auto * p2D = new Vec2f[P.size()];
		p2D[0] = Vec2f(0,0);
		float x = std::sqrt(std::pow(P[1].x-P[0].x,2.0)+std::pow(P[1].y-P[0].y,2.0)+std::pow(P[1].z-P[0].z,2.0));
		std::cout << x << "\n";
		p2D[1] = Vec2f(x,0);
		if(P.size() == 2){
			//super easy, it's just a line :)
			x = std::ceil(x);
			result.format(1, type(), x);
			std::vector<Vec3f> space = linspace(P[0], P[1], x);
			for (unsigned j = 0; j < space.size(); j++){
				Vec3f point = space[j];
				float temp[1] = {0};
				if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  width()* mVoxWidth[2] && point.y <=  height()* mVoxWidth[1] && point.z <=  depth()* mVoxWidth[2]){
					Vec3f p = Vec3f(point.x/ mVoxWidth[0],point.y/ mVoxWidth[1],point.z/ mVoxWidth[2]);
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
			int aDirection = std::ceil(maxA2D - minA2D);
			int oDirection = std::ceil(maxO2D - minO2D);
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
					if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  width()* mVoxWidth[2] && point.y <=  height()* mVoxWidth[1] && point.z <=  depth()* mVoxWidth[2]){
						Vec3f p = Vec3f(point.x/ mVoxWidth[0],point.y/ mVoxWidth[1],point.z/ mVoxWidth[2]);
						read_interp(temp, p);
					}
					result.write(temp,i,j);
				}
			}
		}
		delete [] p2D;
	}
	else if(P.size() == 1){
		//Intersects at one point, this is super easy!
		//calculate point and return array with single point
		result.format(1, type(),1);
		auto point = P[0];
		float temp[1] = {0};
		if(point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  width()* mVoxWidth[2] && point.y <=  height()* mVoxWidth[1] && point.z <=  depth()* mVoxWidth[2]){
			auto p = Vec3f(point.x/ mVoxWidth[0],point.y/ mVoxWidth[1],point.z/ mVoxWidth[2]);
			read_interp(temp, p);
		}
		result.write(temp,0);
	}
	//  for (int i = 0; i < finalPointList.size(); i++){
	//    finalPointList[i] = finalPointList[i]/Vec3f(W,H,D);
	//  }
	//  std::cout << "wtf" << "\n";
	return result;
}


} // al::
