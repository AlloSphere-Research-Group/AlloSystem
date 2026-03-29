#ifndef INC_AL_VOXELS_HPP
#define INC_AL_VOXELS_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Structure for storing scientific volumetric data

	Author(s):
	Matt Wright, 2015, matt@create.ucsb.edu
	Kon Hyong Kim, 2014, konhyong@gmail.com
*/

#include <cstdio> // FILE
#include <cstdint> // int16_t, int32_t, ...
#include <string>
#include <vector>
#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Array.hpp"

namespace al {

typedef int UnitsTy;

enum VoxelUnits {
	VOX_PICOMETERS = -12,
	VOX_ANGSTROMS = -10,
	VOX_NANOMETERS = -9,
	VOX_MICROMETERS = -6,
	VOX_MILLIMETERS = -3,
	VOX_CENTIMETERS = -2,
	VOX_METERS = 0,
	VOX_KILOMETERS = 3
};

enum MRCMode {
	MRC_IMAGE_SINT8 = 0,    //image : signed 8-bit bytes range -128 to 127
	MRC_IMAGE_SINT16 = 1,   //image : 16-bit halfwords
	MRC_IMAGE_FLOAT32 = 2,    //image : 32-bit reals
	MRC_TRANSFORM_INT16 = 3,  //transform : complex 16-bit integers
	MRC_TRANSFORM_FLOAT32 = 4,  //transform : complex 32-bit reals
	MRC_IMAGE_UINT16 = 6        //image : unsigned 16-bit range 0 to 65535
};

struct MRCHeader {
	// @see http://bio3d.colorado.edu/imod/doc/mrc_format.txt

	int32_t nx;			///<  Number of columns
	int32_t ny;			///<  Number of rows
	int32_t nz;			///<  Number of sections
	int32_t mode;		///<  Given by #define MRC_MODE...

	int32_t nxstart;	///<  Starting point of sub image
	int32_t nystart;
	int32_t nzstart;

	int32_t mx;			///< Number of intervals along x,y,z
	int32_t my;
	int32_t mz;

	float xlen;			///< Cell dimensions in angstroms (MRC2014 standard)
	float ylen;
	float zlen;

	float alpha;		///< Cell angles
	float beta;
	float gamma;

	int32_t mapx;		///< Map coloumn 1=x,2=y,3=z
	int32_t mapy;		///< Map row     1=x,2=y,3=z
	int32_t mapz;		///< Map section 1=x,2=y,3=z

	float amin;			///< Minimum pixel value
	float amax;			///< Maximum pixel value
	float amean;		///< Mean pixel value

	int16_t ispg;       ///< Image type
	int16_t nsymbt;     ///< Space group number

	// IMOD-SPECIFIC
	int32_t next;
	int16_t creatid;	///< Used to be creator id, hvem = 1000, now 0
	char blank[30];
	int16_t nint;
	int16_t nreal;
	int16_t sub;
	int16_t zfac;
	float min2;
	float max2;
	float min3;
	float max3;
	int32_t imodStamp;
	int32_t imodFlags;
	int16_t idtype;
	int16_t lens;
	int16_t nd1;		///< Divide by 100 to get float value
	int16_t nd2;
	int16_t vd1;
	int16_t vd2;
	float tiltangles[6];	///< 0,1,2 = original:  3,4,5 = current

	// MRC 2000 standard
	float origin[3];
	char cmap[4];			///< Contains "MAP " for LE, " PAM" for BE
	char machinestamp[4];	///< Little Endian : 68 65 17 17 // Big Endian : 17 17 65 68
	float rms;				///< RMS deviation of densities from mean density

	int32_t nlabl;  		///< Number of labels
	char labels[10][80];
};

/// Structure for storing volumetric data

/// The class contains physical metadata and supports loading data from the MRC
/// file format.
/// \ingroup allocore
class Voxels : public Array {
public:
	Voxels();

	/// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid with units
	Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez, UnitsTy units = VOX_METERS);

	/// Construct dimx x dimy x dimz voxel grid giving dimension of each voxel cube with units
	Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float voxelsize = 1.f, UnitsTy units = VOX_METERS);


	void init(float voxWidthX, float voxWidthY, float voxWidthZ, UnitsTy units);

	float getVoxWidth(unsigned int axis) const;
	void setVoxWidth(unsigned int axis, float voxWidth);

	std::string printVoxWidth(unsigned int axis) const;

	UnitsTy getUnits() const { return mUnits; }
	void setUnits(UnitsTy units){ mUnits = units; }

	std::string printUnits() const;
	std::string printUnits(UnitsTy t) const;

	// functions to support MRC
	MRCHeader& parseMRC(const char * data);

	bool loadFromMRC(std::string filename, bool update = false);

	bool loadFromMRC(std::string filename, UnitsTy ty, float voxWidth);

	bool loadFromMRC(std::string filename, UnitsTy ty, float voxWidthX, float voxWidthY, float voxWidthZ);

	//functions for loading from images
	bool getdir(std::string dir, std::vector<std::string>& files);

	bool parseInfo(std::string dir, std::vector<std::string>& data);

	bool loadFromDirectory(std::string dir);

	//functions for slicing
	bool linePlaneIntersection(const Vec3f& P0, const Vec3f& P1, const Vec3f& planeCenter, const Vec3f& planeNormal, Vec3f * intersection);

	std::vector<Vec3f> linspace(Vec3f a, Vec3f b, int n);

	Vec3f point2Dto3D(Vec3f Q, Vec3f H, Vec3f K, float u, float v);

	bool parallelLinespace(
		Vec3f p0, Vec3f p1, Vec3f p2, Vec3f p3,
		std::vector<Vec3f> &list, std::vector<Vec3f> &list2,
		float aDirection, float oDirection,
		std::vector<Vec3f> &points
	);

	Array slice(Vec3f planeCenter, Vec3f planeNormal, std::vector<Vec3f>& finalPointList);

	// mostly for saving partial changes into mrc header.
	bool writeToMRC(std::string filename, MRCHeader& header);

	// write/read files for voxel class
	bool writeToFile(std::string filename);

	bool loadFromFile(std::string filename);

	void print(FILE * fp = stdout) const;

	float min() const { return mMin; }

	float max() const { return mMax; }

	float mean() const { return mMean; }

	float rms() const { return mRMS; }

protected:
	UnitsTy mUnits;
	float mVoxWidth[3];
	float mMin, mMax, mMean, mRMS;
};

} // al::
#endif
