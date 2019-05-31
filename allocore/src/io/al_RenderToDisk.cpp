#include <cstdlib> // std::system
#include "allocore/io/al_RenderToDisk.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Time.hpp"
#include "allocore/types/al_Conversion.hpp"

namespace al{

static void serializeToBigEndian(char * out, uint32_t in){
	out[0] = (in >> 24) & 0xff;
	out[1] = (in >> 16) & 0xff;
	out[2] = (in >>  8) & 0xff;
	out[3] = (in      ) & 0xff;
}

static void serializeToBigEndian(char * out, float in){
	union{ uint32_t u; float f; } u;
	u.f = in;
	serializeToBigEndian(out, u.u);
}


RenderToDisk::RenderToDisk(Mode m)
:	mMode(m), mFrameNumber(0), mElapsedSec(0),
	mGraphicsBuf(-1),
	mImageExt("png"), mImageCompress(50),
	mActive(false), mWroteImages(false), mWroteAudio(false)
{
	mPBOs[0] = 0;
	resetPBOQueue();

	/*AudioRing ring;
	ring.resize(2,2,4);

	for(int j=0; j<8; ++j){
		float in[4] = {2*j,2*j+1, 20*j,20*j+10};
		float out[4] = {0,0,0,0};

		ring.write(in);
		ring.read(out);
		for(int i=0; i<4; ++i) printf("%f\n", out[i]);
	}*/
}

RenderToDisk::~RenderToDisk(){
	stop();
}

RenderToDisk& RenderToDisk::mode(Mode v){
	if(!mActive){
		mMode = v;
	}
	return *this;
}

RenderToDisk& RenderToDisk::path(const std::string& v){
	mUserPath = v;
	return *this;
}

RenderToDisk& RenderToDisk::imageFormat(const std::string& ext, int compress){
	mImageExt = ext;
	mImageCompress = compress;
	return *this;
}

RenderToDisk& RenderToDisk::audioGain(float v){
	mAudioRing.mGain = v;
	return *this;
}


bool RenderToDisk::toggle(al::AudioIO& aio, al::Window& win, double fps){
	return toggle(&aio, &win, fps);
}
bool RenderToDisk::toggle(al::AudioIO& aio){
	return toggle(&aio, 0);
}
bool RenderToDisk::toggle(al::Window& win, double fps){
	return toggle(0, &win, fps);
}

bool RenderToDisk::toggle(al::AudioIO * aio, al::Window * win, double fps){
	if(mActive){
		stop();
		return true;
	}
	else{
		return start(aio, win, fps);
	}
}

bool RenderToDisk::start(al::AudioIO& aio, al::Window& win, double fps){
	return start(&aio, &win, fps);
}
bool RenderToDisk::start(al::AudioIO& aio){
	return start(&aio, 0);
}
bool RenderToDisk::start(al::Window& win, double fps){
	return start(0, &win, fps);
}

bool RenderToDisk::start(al::AudioIO * aio, al::Window * win, double fps){
	if(mActive) return true;

	if(NON_REAL_TIME == mMode && 0 == win){
		fprintf(stderr, "RenderToDisk::start: Warning-- Non-real-time audio-only rendering currently not supported\n");
		return false;
	}

	mWroteImages = mWroteAudio = false;
	mFrameNumber = 0;

	// Make path on HD
	makePath();

	if(aio){
		// Open sound file for writing
		mSoundFile.open((mPath + "/output.au").c_str(), std::ofstream::out | std::ofstream::binary);
	
		if(!mSoundFile.is_open()) return false;
	
		// Write AU header to file:
		//   magic, data offset, data size, sample type (6=float), sample rate, channels
		// Reference:
		//	http://pubs.opengroup.org/external/auformat.html
		//	http://paulbourke.net/dataformats/audio/
		char hdr[24] =
			{'.','s','n','d', 0,0,0,24, -1,-1,-1,-1, 0,0,0,6, 0,0,0,0, 0,0,0,0};
		serializeToBigEndian(hdr + 16, uint32_t(aio->framesPerSecond()));
		serializeToBigEndian(hdr + 20, uint32_t(aio->channelsOut()));
		mSoundFile.write(hdr, sizeof(hdr));
	
		// Resize audio buffer to hold one block
		//int bytesPerSample = 4;
		//mAudioBuf.resize(aio.channelsOut() * aio.framesPerBuffer() * bytesPerSample);

		unsigned ringSizeInFrames = aio->fps() * 0.25; // 1/4 second of audio
		unsigned numBlocks = ringSizeInFrames/aio->framesPerBuffer();
		if(numBlocks < 2) numBlocks = 2; // should buffer at least two (?) blocks
		mAudioRing.resize(aio->channelsOut(), aio->framesPerBuffer(), numBlocks);
	}

	mAudioIO = aio;
	mWindow = win;

	if(mWindow){
		mWindowFPS = mWindow->fps();
		mFrameDur = 1. / (fps>0 ? fps : mWindowFPS);
	}
	else{
		mFrameDur = mAudioIO->secondsPerBuffer();
	}
	
	mActive = true;

	if(mAudioIO){
		struct F{ static void * threadFunc(void * user){
			RenderToDisk& outer = *(RenderToDisk*)(user);
	
			while(outer.mActive){
				//printf("SoundFile writer thread\n");

				const int readCode = outer.mAudioRing.read();
				if(readCode){
					//printf("SoundFile writer thread: %s\n", readCode>0 ? "read" : "underrun");
					if(readCode<0) fprintf(stderr, "SoundFile writer thread: underrun\n");
					outer.mSoundFile.write(
						reinterpret_cast<const char*>(outer.mAudioRing.readBuffer()),
						outer.mAudioRing.blockSizeInSamples() * sizeof(float)
					);
				}
				else{
					//printf("SoundFile writer thread: overrun (sleeping...)\n");
					al_sleep(0.01);
				}
			}
			return NULL;
		}};

		mSoundFileThread.start(F::threadFunc, this);

		if(NON_REAL_TIME == mMode){
			mAudioIO->stop();
		}

		mAudioIO->append(*this);
	}

	if(mWindow){
		mWindow->append(*this);

		if(NON_REAL_TIME == mMode){
			mWindow->asap(true);
			mWindow->vsync(false);
		}
		else{
			mWindow->fps(1./mFrameDur);
		}
	}

	return true;
}

void RenderToDisk::stop(){
	if(!mActive) return;
	
	mActive = false;

	if(mWindow){
		// Empty and reset PBO queue
		for(int i=0; i<Npbos; ++i) writeImage();
		resetPBOQueue();

		mWroteImages = true;

		if(0 != mPBOs[0]){
			glDeleteBuffers(Npbos, mPBOs);
			mPBOs[0] = 0;
		}

		mWindow->remove(*this);

		if(NON_REAL_TIME == mMode){
			mWindow->asap(false);
			mWindow->vsync(true);
		}
		else{
			mWindow->fps(mWindowFPS);
		}

		mWindow = 0;
	}

	if(mAudioIO){
		mSoundFileThread.join();
		mSoundFile.close();

		mWroteAudio = true;

		mAudioIO->remove(*this);
	
		if(NON_REAL_TIME == mMode){
			mAudioIO->start();
		}

		mAudioIO = 0;
	}
}


void RenderToDisk::saveScreenshot(unsigned w, unsigned h, unsigned l, unsigned b){
	makePath();
	saveImage(w,h,l,b, /*usePBO=*/false);
}

void RenderToDisk::saveScreenshot(al::Window& win){
	saveScreenshot(win.width(), win.height());
}


void RenderToDisk::makePath(){
	// If no user path specified, create default with time string
	if(mUserPath.empty()){
		mPath = "./render";
		mPath += al::toString((unsigned long long)(al::timeNow()*1000) % 31536000000ull);
	}
	else{
		mPath = mUserPath;
	}

	// Create output directory if it doesn't exist
	if(!File::exists(mPath)) Dir::make(mPath);
}

void RenderToDisk::write(){
	writeImage();
	writeAudio();
}

void RenderToDisk::onAudioCB(AudioIOData& io){
	mAudioRing.write(io.outBuffer(0));
}

bool RenderToDisk::onFrame(){
	write();
	return true;
}

void RenderToDisk::writeAudio(){

	if(!mActive || (REAL_TIME == mMode) || !mAudioIO) return;

	// Compute number of blocks over one frame of graphics
	unsigned audioBlock0 = mFrameNumber * mFrameDur / mAudioIO->secondsPerBuffer();
	unsigned audioBlock1 = (mFrameNumber+1) * mFrameDur / mAudioIO->secondsPerBuffer();
	unsigned audioBlocks = audioBlock1 - audioBlock0;

	//unsigned Nchans = mAudioIO->channelsOut();
	//unsigned Nblock = mAudioIO->framesPerBuffer();
	
	for(unsigned k=0; k<audioBlocks; ++k){

		// In non-real-time mode, this may write many blocks of audio. The read
		// thread must check for new samples often enough to avoid underruns.
		mAudioIO->processAudio();
		
		/*
		// Interleave and big-endianize samples
		for(unsigned c=0; c<Nchans; ++c){
			for(unsigned f=0; f<Nblock; ++f){
				float smp = mAudioIO->out(c,f);
				serializeToBigEndian(&mAudioBuf[(f*Nchans + c)*4], smp);
			}
		}

		// Write to file
		mSoundFile.write(&mAudioBuf[0], mAudioBuf.size());
		//*/
	}
}

void RenderToDisk::writeImage(){
	if(!mActive || !mWindow) return;
	saveImage(mWindow->width(), mWindow->height());
}

void RenderToDisk::resetPBOQueue(){
	mPBOIdx = 0;
	mReadPBO = false;
}

void RenderToDisk::saveImage(
	unsigned w, unsigned h, unsigned l, unsigned b, bool usePBO
){
	const auto numBytes = w*h*3;
	if(numBytes > mPixels.size()) mPixels.resize(numBytes);

	unsigned char * pixs = &mPixels[0];

	#ifdef AL_GRAPHICS_SUPPORTS_SET_RW_BUFFERS
	// Set read buffer
	//glReadBuffer(GL_COLOR_ATTACHMENT0); // for FBO
	//glReadBuffer(GL_BACK);
	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	/*
	GLint drawBuffer;
	glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
	glReadBuffer(drawBuffer);
	//*/
	if(mGraphicsBuf != GLenum(-1)){
		glReadBuffer(mGraphicsBuf);
	}
	#endif

	bool readPixels = false;

	#ifdef AL_GRAPHICS_SUPPORTS_PBO
	/* Copy pixels out of framebuffer into client memory.
	A PBO FIFO is used to avoid stalling on glReadPixels. See:
	http://www.roxlu.com/2014/048/fast-pixel-transfers-with-pixel-buffer-objects
	https://vec.io/posts/faster-alternatives-to-glreadpixels-and-glteximage2d-in-opengl-es
	http://www.opengl.org/wiki/Pixel_Buffer_Object
	http://www.gamedev.net/topic/575590-real-time-opengl-screen-capture/
	http://stackoverflow.com/questions/12157646/how-to-render-offscreen-on-opengl
	*/
	if(usePBO){
		if(0 == mPBOs[0]){ // create PBOs
			glGenBuffers(Npbos, mPBOs);
			for(auto& pbo : mPBOs){
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
				glBufferData(GL_PIXEL_PACK_BUFFER, numBytes, NULL, GL_STREAM_READ);
			}
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}

		//printf("PBO %d %s\n", mPBOIdx, mReadPBO ? "(read back)" : "");
		auto pbo = mPBOs[mPBOIdx];
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);

		if(mReadPBO){
			// Get pointer to data in currently bound PBO
			// (This will block until glReadPixels finishes from last time the PBO was bound)
			auto * ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			memcpy(pixs, ptr, numBytes);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			readPixels = true;
		}

		// This will perform asynchronously into the bound PBO
		glReadPixels(l,b,w,h, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		mPBOIdx = (mPBOIdx + 1) % Npbos;
		mReadPBO = mReadPBO || (mPBOIdx == 0); // written to all PBOs at least once
	}
	else
	#endif
	{	// This will block until all draw commands finish
		glReadPixels(l,b,w,h, GL_RGB, GL_UNSIGNED_BYTE, pixs);
		readPixels = true;
	}


	// Launch thread to write pixels out to an image file
	if(readPixels){
		//printf("----\nWriting frame %d\n", mFrameNumber);

		// At 40 FPS: 60 x 60 x 40 = 144000 frames/hour
		std::string name = mPath + "/" + al::toString("%07u", mFrameNumber) + "." + mImageExt;
		Image::Format format = Image::RGB;

		CHECK_THREADS:
		int i=0;
		for(; i<Nthreads; ++i){
			//printf("Checking if image writer %d is free ...\n", i);
			if(mImageWriters[i].run(name, mPixels, w,h, format, mImageCompress)){
				//printf("Running image writer %d\n", i);
				break;
			}
		}
		// All threaded image writers were busy
		if(i == Nthreads){
			//printf("Error: all image writers are busy...\n");
			al::wait(1./1e3); // wait 1 ms for disk i/o to finish up (hopefully)
			goto CHECK_THREADS;
			// Use main thread---this will definitely stall
			//al::Image::save(name, pixs, w,h, format, mImageCompress);
		}
	
		++mFrameNumber;
	}
}

void RenderToDisk::createVideo(int videoCompress, int videoEncodeSpeed){

	// Nothing to do without image sequence
	if(!mWroteImages) return;

	std::string prog;
	#ifdef AL_WINDOWS
		// Note: path must be DOS style for std::system
		prog = "c:\\Program Files\\ffmpeg\\bin\\ffmpeg";
	#else
		prog = "ffmpeg";
	#endif

	std::string args;
	args += " -r " + al::toString(1./mFrameDur);
	args += " -i " + path() + "/%07d." + mImageExt;
	if(mWroteAudio){
		args += " -i " + path() + "/output.au -c:a aac -b:a 192k";
	}

	//args += " -pix_fmt yuv420p" // for compatibility with outdated media players
	//args += " -crf 20 -preset slower";
	args += " -crf " + al::toString(videoCompress);
	static const std::string speedStrings[] = {"placebo","veryslow","slower","slow","medium","fast","faster","veryfast","superfast","ultrafast"};
	args += " -preset " + speedStrings[videoEncodeSpeed];
	args += " " + path() + "/movie.mp4";

	std::string cmd = "\"" + prog + "\"" + args;
	//printf("%s\n", cmd.c_str());

	// TODO: thread this; std::system blocks until the command finishes
	std::system(cmd.c_str());
}


RenderToDisk::AudioRing::AudioRing()
:	mChannels(0), mBlockSize(0), mNumBlocks(0), mWriteBlock(0), mReadBlock(0)
{}

void RenderToDisk::AudioRing::resize(
	unsigned channels, unsigned blockSize, unsigned numBlocks
){
	mChannels  = channels;
	mBlockSize = blockSize;
	mNumBlocks = numBlocks;

	// Note: last block is for read buffer
	mBuffer.resize(channels * blockSize * (numBlocks + 1));
}

void RenderToDisk::AudioRing::write(const float * block){
	unsigned blockSamps = blockSizeInSamples();
	unsigned wblock = mWriteBlock % mNumBlocks;
	memcpy(&mBuffer[wblock * blockSamps], block, blockSamps * sizeof(mBuffer[0]));
	++mWriteBlock;
}

int RenderToDisk::AudioRing::read(){

	//printf("RenderToDisk::AudioRing::read: r=%d, w=%d\n", mReadBlock, mWriteBlock);

	// Overrun (in general, good)
	if(mReadBlock == mWriteBlock) return 0;

	int returnCode = 1;

	// Underrun (this is bad)
	if((mWriteBlock - mReadBlock) >= mNumBlocks){
		//fprintf(stderr, "AudioRing::read underrun (addr=%p)\n", this);

		// Copy write position since write thread may change it
		unsigned writeBlock = mWriteBlock;

		// Set read block to oldest block
		if(writeBlock >= mNumBlocks) mReadBlock = writeBlock - mNumBlocks;

		returnCode = -1;
	}

	unsigned rblock = mReadBlock % mNumBlocks;
	const float * src = &mBuffer[rblock * blockSizeInSamples()];
	float * dst = &mBuffer[blockSizeInSamples() * mNumBlocks];

	// Copy samples into read buffer as fast as possible
	for(unsigned c=0; c<mChannels; ++c){
		for(unsigned i=0; i<mBlockSize; ++i){
			dst[i*mChannels + c] = (*src++) * mGain;
		}
	}

	// Big-endianize
	if(true){
		for(unsigned i=0; i<blockSizeInSamples(); ++i){
			float& s = dst[i];
			serializeToBigEndian(reinterpret_cast<char*>(&s), s);
		}
	}

	++mReadBlock;

	return returnCode;
}

const float * RenderToDisk::AudioRing::readBuffer() const {
	return &mBuffer[blockSizeInSamples() * mNumBlocks];
}

unsigned RenderToDisk::AudioRing::blockSizeInSamples() const {
	return mChannels * mBlockSize;
}



bool RenderToDisk::ImageWriter::run(
	const std::string& path,
	const std::vector<unsigned char>& pixels, unsigned w, unsigned h,
	Image::Format format, unsigned compress
){
	if(mBusy) return false;

	mBusy = true;

	// Create local copy of pixels
	mImage.resize<unsigned char>(w,h, format);
	memcpy(mImage.pixels<void>(), &pixels[0], pixels.size());

	mImage.compression(compress);
	mPath = path;

	// Wait for any thread function in progress
	mThread.join();

	struct F{
		static void * threadFunc(void * user){
			ImageWriter& outer = *(ImageWriter*)(user);
			//outer.mBusy = true;
			outer.mImage.save(outer.mPath);
			outer.mBusy = false;
			return NULL;
		}
	};

	mThread.start(F::threadFunc, this);

	return true;
}

}
