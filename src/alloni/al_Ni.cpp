#include "alloni/al_Ni.hpp"


#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <XnCppWrapper.h>
//#include <usb.h>

using namespace al;

xn::Context context;
XnStatus status;

std::vector<xn::NodeInfo> device_nodes, depth_nodes, image_nodes;
std::map< unsigned char, std::map<unsigned char, unsigned > > bus_map;
//static bool initialized = false;


static bool ok(XnStatus status) {
	if (status != XN_STATUS_OK) {
		printf("OpenNI error: %s\n", xnGetStatusString(status));
		return false;
	}
	return true;
}


Ni& Ni :: get() {
	static Ni singleton;
	return singleton;
}


xn::NodeInfoList device_node_info_list, depth_node_info_list;

Ni :: Ni() {
	int i;

	// enumerate devices:
	if (!ok(context.Init())) return;

	printf("created Ni\n");


	// enumerate all devices
	printf("Searching for OpenNI compliant USB devices:\n");
	if (!ok(context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, device_node_info_list))) return;

	printf("listing devices\n");
	i=0;
	for (xn::NodeInfoList::Iterator nodeIt = device_node_info_list.Begin (); nodeIt != device_node_info_list.End (); ++nodeIt, ++i) {
		const xn::NodeInfo& info = *nodeIt;
		const XnProductionNodeDescription& description = info.GetDescription();
		unsigned short vendor_id;
		unsigned short product_id;
		unsigned char bus;
		unsigned char address;
		sscanf(info.GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
		std::string connection_string = info.GetCreationInfo();
		std::transform (connection_string.begin (), connection_string.end (), connection_string.begin (), std::towlower);
		printf(">%2d (USB bus %2i address %2i): vendor %s (vendor_id %i) name %s (product_id %i), connection string %s\n",
			i, bus, address,
			description.strVendor, vendor_id,
			description.strName, product_id,
			connection_string.c_str());

		if (connection_string.substr (0,4) == "045e") {
			printf("\t(Kinect)\n");
		}

		device_nodes.push_back(info);
		bus_map[bus][address] = device_nodes.size();
	}

	depth_nodes.clear();
	if (!ok(context.EnumerateProductionTrees(XN_NODE_TYPE_DEPTH, NULL, depth_node_info_list))) return;
	i=0;
	for (xn::NodeInfoList::Iterator nodeIt = depth_node_info_list.Begin (); nodeIt != depth_node_info_list.End (); ++nodeIt, ++i) {
		xn::NodeInfo info = *nodeIt;
		const XnProductionNodeDescription& description = info.GetDescription();
		printf("depth %2d: vendor %s name %s, connection %s\n", i, description.strVendor, description.strName, info.GetCreationInfo());
		depth_nodes.push_back(info);
	}
	printf("Initialized OpenNI\n");
}

struct Kinect :: Impl {

	Impl(xn::NodeInfo info) {
		hasData = false;
		if (!ok(context.CreateProductionTree(info))) return;
		if (!ok(info.GetInstance(mDepthGenerator))) return;

		XnMapOutputMode mode;
		mode.nXRes = 640;
		mode.nYRes = 480;
		mode.nFPS = 30;
		mDepthGenerator.SetMapOutputMode(mode);
		mDepthGenerator.SetIntProperty ("RegistrationType", 2);	// 2 for kinect, 1 for primesense

		mDepthGenerator.RegisterToNewDataAvailable(NewDepthDataAvailable, this, mDepthCallbackHandle);

	}

	~Impl() {
		mDepthGenerator.UnregisterFromNewDataAvailable(mDepthCallbackHandle);
	}

	static void NewDepthDataAvailable (xn::ProductionNode& node, void* cookie) {
		Impl * self = (Impl *)cookie;
		//printf("%p\n", cookie);
		//printf("new depth data %p\n", cookie);
		// trigger condition:
		self->hasData = true;
	}

	bool getMetaData() {
		//XnUInt64 timestamp;
		hasData = false;
		//printf("!");
		//if (mDepthGenerator.IsNewDataAvailable(&timestamp)) {
			//if (ok(context.WaitOneUpdateAll(mDepthGenerator))) {	// nice fps on osx
			//if (ok(context.WaitAndUpdateAll())) {
			if (ok(mDepthGenerator.WaitAndUpdateData())) {
				//printf("%p\n", this);
				mDepthGenerator.GetMetaData(mDepthMD);
				return true;
			}
		//}
		return false;
	}

	xn::DepthGenerator mDepthGenerator;
	xn::DepthMetaData mDepthMD;
  	XnCallbackHandle mDepthCallbackHandle;
	bool hasData;
};

Kinect :: Kinect(unsigned deviceID)
:	mImpl(NULL),
	mDepthArray(1, AlloFloat32Ty, 640, 480),
	mRawDepthArray(1, AlloFloat32Ty, 640, 480),
	mRealWorldArray(3, AlloFloat32Ty, 640, 480),
	mTime(0),
	mDepthNormalize(true),
	mFPS(0),
	mDeviceID(deviceID),
	mZPD(120),
	mZPPS(0.104200)
{
	printf("created Kinect\n");
}

Kinect :: ~Kinect() {
	if (mImpl) delete mImpl;
}

void * Kinect :: getContext() {
	Ni::get();
	return (void *)&context;
}

bool Kinect :: start() {
	return mThread.start(threadFunction, this);
}

bool Kinect :: stop() {
	if (mImpl == 0) return false;
	mActive = false;
	return mThread.join();
}


//void Kinect :: toRealWorld(unsigned count, Vec3f pos[], Vec3f res[]) const {
//	if (mImpl) mImpl->mDepthGenerator.ConvertProjectiveToRealWorld(count, (const XnPoint3D *)pos, (XnPoint3D *)res);
//}

//void Kinect :: toProjective(unsigned count, Vec3f pos[], Vec3f res[]) const {
//	if (mImpl) mImpl->mDepthGenerator.ConvertRealWorldToProjective(count, (const XnPoint3D *)pos, (XnPoint3D *)res);
//}

bool Kinect :: tick() {
	if(mImpl->hasData && mImpl->getMetaData()) {
		const XnUInt xres = mImpl->mDepthMD.XRes();
		const XnUInt yres = mImpl->mDepthMD.YRes();
		const XnDepthPixel zres = mImpl->mDepthMD.ZRes();
		const XnDepthPixel* pDepth = mImpl->mDepthMD.Data();

		const float zscale = mDepthNormalize ? 1.f/zres : 1.f;
		const float zoffset = 0.f;

		//printf("%p: %ix%ix%i, %f fps\n", this, xres, yres, (int)zres, mFPS);

		// copy into Array:
		float * optr = (float *)mDepthArray.data.ptr;
		float * rptr = (float *)mRawDepthArray.data.ptr;
		//for (unsigned i=0; i<xres*yres; i++) {

		for (unsigned y=0; y<yres; y++) {
			for (unsigned x=0; x<xres; x++) {
				XnDepthPixel D = (*pDepth++);
				*rptr++ = D;
				*optr++ = D * zscale + zoffset;
				Vec3d p(toRealWorld(x, y, D));
				mRealWorldArray.write(p.elems, x, y);
			}
		}

		// update FPS:
		al_sec when = al_time();
		al_sec dt = when - mTime;
		mTime = when;
		mFPS = 1./dt;

		// callbacks:
		for (std::list<Callback *>::iterator it = mCallbacks.begin(); it != mCallbacks.end(); it++) {
			(*it)->onKinectData(*this);
		}
	}

	return true;
}

void * Kinect :: threadFunction(void * userData) {
	Kinect& self = *(Kinect *)userData;

	// find the info for this device:
	printf("started Kinect thread\n");
	Ni::get();
	if (self.mDeviceID < depth_nodes.size()) {
		xn::NodeInfo info = depth_nodes[self.mDeviceID % depth_nodes.size()];
		self.mImpl = new Impl(info);

		if (self.mImpl) {
			printf("start generating kinect... %p\n", userData);
			XnStatus s = self.mImpl->mDepthGenerator.StartGenerating();
			if (!ok(s)) return 0;


			//self.mImpl->mDepthGenerator.GetIntProperty("ZPD", (XnUInt64&)self.mZPD);
			//self.mImpl->mDepthGenerator.GetRealProperty("ZPPS", self.mZPPS);

			self.mTime = al_time();
			self.mActive = true;

			self.mImpl->hasData = false;

			while (self.mActive && self.tick()) { al_sleep(0.010); }

			ok(self.mImpl->mDepthGenerator.StopGenerating());
		}

		delete self.mImpl;
		self.mImpl = NULL;
	}
	return 0;
}
