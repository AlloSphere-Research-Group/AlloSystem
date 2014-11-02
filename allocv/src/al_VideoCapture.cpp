#include "allocv/al_VideoCapture.hpp"

/*
From opencv2/highgui/highgui.hpp:

class CV_EXPORTS_W VideoCapture
{
public:
    CV_WRAP VideoCapture();
    CV_WRAP VideoCapture(const string& filename);
    CV_WRAP VideoCapture(int device);

    virtual ~VideoCapture();
    CV_WRAP virtual bool open(const string& filename);
    CV_WRAP virtual bool open(int device);
    CV_WRAP virtual bool isOpened() const;
    CV_WRAP virtual void release();

	// Grabs the next frame from video file or capturing device.
    CV_WRAP virtual bool grab();
	
	// Decodes and returns the grabbed video frame.
    CV_WRAP virtual bool retrieve(CV_OUT Mat& image, int channel=0);
	
	// Alias of read()
    virtual VideoCapture& operator >> (CV_OUT Mat& image);
	
	// Grabs, decodes and returns the next video frame.
    CV_WRAP virtual bool read(CV_OUT Mat& image);

    CV_WRAP virtual bool set(int propId, double value);
    CV_WRAP virtual double get(int propId);

protected:
    Ptr<CvCapture> cap;
};

*/

namespace al{


VideoCapture::VideoCapture()
:	mFPS(1.), mRate(1.), mIsFile(false)
{}


bool VideoCapture::open(const std::string& filename){
	if(cvVideoCapture.open(filename)){
		mIsFile = true;
		mFPS = get(CV_CAP_PROP_FPS);
		return true;
	}
	return false;
}

bool VideoCapture::open(int device){
	if(cvVideoCapture.open(device)){
		mIsFile = false;
		mFPS = get(CV_CAP_PROP_FPS);
		if(mFPS == 0.) mFPS = 30.;
		return true;
	}
	return false;
}

void VideoCapture::release(){
	cvVideoCapture.release();
}

bool VideoCapture::grab(){
	return cvVideoCapture.grab();
}

bool VideoCapture::retrieve(Array& dst, int channel, int copyPolicy){
	bool res = cvVideoCapture.retrieve(cvFrame, channel);
	fromCV(dst, cvFrame, copyPolicy);
	return res;
}

bool VideoCapture::retrieveFlip(Array& dst, int channel){
	return retrieve(dst, channel, -1);
}

bool VideoCapture::read(Array& dst, int copyPolicy){
	bool res = cvVideoCapture.read(cvFrame);
	fromCV(dst, cvFrame, copyPolicy);
	return res;
}

bool VideoCapture::set(int cvCapProp, double val){
	return cvVideoCapture.set(cvCapProp,val);
}

VideoCapture& VideoCapture::posMsec(double msec){
	set(CV_CAP_PROP_POS_MSEC, msec);
	return *this;
}

VideoCapture& VideoCapture::posFrames(double frame){
	set(CV_CAP_PROP_POS_FRAMES, frame);
	return *this;
}

VideoCapture& VideoCapture::posFrac(double frac){
	set(CV_CAP_PROP_POS_AVI_RATIO, frac);
	return *this;
}

VideoCapture& VideoCapture::fps(double val){
	mFPS = val;
	return *this;
}

VideoCapture& VideoCapture::rate(double fpsMul){
	mRate = fpsMul;
	return *this;
}


bool VideoCapture::isOpened() const {
	return cvVideoCapture.isOpened();
}

double VideoCapture::get(int cvCapProp) const {
	return cvVideoCapture.get(cvCapProp);
}

double VideoCapture::fps() const {
	return mFPS;
}

double VideoCapture::numFrames() const {
	return get(CV_CAP_PROP_FRAME_COUNT);
}

double VideoCapture::rate() const {
	return mRate;
}

double VideoCapture::width() const {
	return get(CV_CAP_PROP_FRAME_WIDTH);
}

double VideoCapture::height() const {
	return get(CV_CAP_PROP_FRAME_HEIGHT);
}

double VideoCapture::aspect() const {
	double w = width();
	double h = height();
	return (h!=0. && w!=0.) ? w/h : 1.;
}

bool VideoCapture::rgb() const {
	return get(CV_CAP_PROP_CONVERT_RGB);
}

int VideoCapture::fourcc() const {
	return int(get(CV_CAP_PROP_FOURCC));
}

std::string VideoCapture::fourccString() const {
	union{ int i; char c[4]; } x = { fourcc() };
	return std::string(x.c, 4);
}

double VideoCapture::posFrames() const {
	return get(CV_CAP_PROP_POS_FRAMES);
}

void VideoCapture::loop(double minFrame, double maxFrame){
	double Nf = numFrames();
	if(maxFrame < 0) maxFrame += Nf + 1.;
	else if(maxFrame > Nf) maxFrame = Nf;
	double pos = posFrames();
	if(pos >= maxFrame) posFrames(minFrame);
}

bool VideoCapture::isFile() const {
	return mIsFile;
}

void VideoCapture::print(FILE * fp){
	fprintf(fp, "%g x %g %s %s, %g fps",
		width(), height(), rgb()?"RGB":"BGR", fourccString().c_str(), fps());
	if(isFile()){
		fprintf(fp, ", %g frames (%g sec)", numFrames(), numFrames()/fps());
	}
	fprintf(fp, "\n");
}




VideoCaptureHandler::VideoThreadFunction::VideoThreadFunction()
: 	videoCapture(NULL), handler(NULL), streamIdx(-1)
{}

void VideoCaptureHandler::VideoThreadFunction::operator()(){
//printf("VideoThreadFunc called\n");
	if(NULL!=videoCapture){
		if(videoCapture->grab()){
			handler->onVideo(*videoCapture, streamIdx);
			double fps = videoCapture->fps() * videoCapture->rate();
			handler->mWorkThreads[streamIdx].thread.period(1./fps);
		}
	}
}


void VideoCaptureHandler::WorkThread::start(){
	//printf("WorkThread::start(): %p %p\n", func.videoCapture, func.handler);
	thread.start(func);
}

void VideoCaptureHandler::WorkThread::stop(){
	thread.stop();
}


VideoCaptureHandler::VideoCaptureHandler(int numStreams){
	numVideoStreams(numStreams);
}

VideoCaptureHandler::~VideoCaptureHandler(){
	stopVideo();
}

VideoCaptureHandler& VideoCaptureHandler::numVideoStreams(int num){
	mWorkThreads.resize(num);
	return *this;
}

int VideoCaptureHandler::numVideoStreams() const {
	return int(mWorkThreads.size());
}

VideoCaptureHandler& VideoCaptureHandler::attach(VideoCapture& vid, int streamIdx){
	if(streamIdx>=0 && streamIdx<numVideoStreams()){
		WorkThread& t = mWorkThreads[streamIdx];
		t.func.handler = this;
		t.func.videoCapture = &vid;
		t.func.streamIdx = streamIdx;
	}
	return *this;
}

void VideoCaptureHandler::startVideo(){
	for(
		WorkThreads::iterator it = mWorkThreads.begin();
		it != mWorkThreads.end();
		++it
	){
		(*it).start();
	}
}

void VideoCaptureHandler::stopVideo(){
	for(
		WorkThreads::iterator it = mWorkThreads.begin();
		it != mWorkThreads.end();
		++it
	){
		(*it).stop();
	}
}

} // al::
