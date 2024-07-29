#include <cstdlib> // std::system
#include <cstring> // memcpy 
#include "allocore/graphics/al_OpenGL.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Time.hpp"
#include "allocore/types/al_Conversion.hpp"
#include "allocore/io/al_RenderToDisk.hpp"

namespace al{

static void serializeToBigEndian(char * out, uint16_t in){
	out[0] = (in >>  8) & 0xff;
	out[1] = (in      ) & 0xff;
}

static void serializeToBigEndian(char * out, uint32_t in){
	out[0] = (in >> 24) & 0xff;
	out[1] = (in >> 16) & 0xff;
	out[2] = (in >>  8) & 0xff;
	out[3] = (in      ) & 0xff;
}

static void serializeToBigEndian(char * out, int16_t in){
	union{ uint16_t u; int16_t s; } u;
	u.s = in;
	serializeToBigEndian(out, u.u);
}

static void serializeToBigEndian(char * out, float in){
	union{ uint32_t u; float f; } u;
	u.f = in;
	serializeToBigEndian(out, u.u);
}


RenderToDisk::RenderToDisk(Mode m)
:	mMode(m)
{
	mPBOs[0] = 0;
	resetPBOQueue();

	mAudioCB = [this](const AudioIOData& io){
		mAudioRing.write(io.bufferOut().data());
	};

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

RenderToDisk& RenderToDisk::dir(const std::string& s){
	mUserDir = s;
	return *this;
}

RenderToDisk& RenderToDisk::imageFormat(const std::string& ext, int compress, int paletteSize){
	mImageExt = ext;
	mImageCompress = compress;
	mImagePaletteSize = paletteSize;
	return *this;
}

RenderToDisk& RenderToDisk::soundFormat(SoundFormat fmt){
	mSoundFormat = fmt;
	return *this;
}

RenderToDisk& RenderToDisk::soundFileName(const std::string& name){
	mSoundFileName = name;
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

	// Make dir on HD
	makeDir();

	if(aio){
		// Open sound file for writing
		auto sfname = mSoundFileName.empty() ? std::string("output") : mSoundFileName;
		mSoundFile.open((mDir + "/" + sfname + ".au").c_str(), std::ofstream::out | std::ofstream::binary);
	
		if(!mSoundFile.is_open()) return false;

		// Write AU header to file:
		//   magic, data offset, data size, sample type (3=PCM16 6=float), sample rate, channels
		// After the header, we write interleaved sample data.
		// The header and data must be written in big endian format.
		// Reference:
		//	http://pubs.opengroup.org/external/auformat.html
		//	http://paulbourke.net/dataformats/audio/

		char fmt;
		unsigned char bytesPerSample;
		switch(mSoundFormat){
		default:
		case FLOAT32: fmt=6; bytesPerSample=4; break;
		case PCM16:   fmt=3; bytesPerSample=2; break;
		}

		int numChans = aio->bufferOut().channels();

		char hdr[24] =
			{'.','s','n','d', 0,0,0,24, -1,-1,-1,-1, 0,0,0,fmt, 0,0,0,0, 0,0,0,0};
		serializeToBigEndian(hdr + 16, uint32_t(aio->framesPerSecond()));
		serializeToBigEndian(hdr + 20, uint32_t(numChans));
		mSoundFile.write(hdr, sizeof(hdr));
	
		// Resize audio buffer to hold one block
		//int bytesPerSample = 4;
		//mAudioBuf.resize(aio.channelsOut() * aio.framesPerBuffer() * bytesPerSample);

		unsigned ringSizeInFrames = aio->fps() * 0.25; // 1/4 second of audio
		unsigned numBlocks = ringSizeInFrames/aio->framesPerBuffer();
		if(numBlocks < 2) numBlocks = 2; // should buffer at least two (?) blocks
		mAudioRing.resize(numChans, aio->framesPerBuffer(), numBlocks);
		mAudioRing.mInputInterleaved = aio->interleaved();
		mSoundFileSamples.resize(aio->framesPerBuffer() * numChans * bytesPerSample);
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

		if(NON_REAL_TIME == mMode){
			mAudioIO->stop();
		}

		mAudioIO->append(mAudioCB);

		mSoundFileThread.start(
			[this](){
				while(mActive){
					//printf("SoundFile writer thread\n");

					const int readCode = mAudioRing.read();
					if(readCode){
						//printf("SoundFile writer thread: %s\n", readCode>0 ? "read" : "underrun");
						if(readCode<0) fprintf(stderr, "SoundFile writer thread: underrun\n");
						const float * src = mAudioRing.readBuffer();
						char * dst = mSoundFileSamples.data();
						int numSamps = mAudioRing.blockSizeInSamples();
						switch(mSoundFormat){
						default:
						case FLOAT32:
							for(int i=0; i<numSamps; ++i){
								serializeToBigEndian(dst + i*4, src[i]);
							}
							break;
						case PCM16:
							for(int i=0; i<numSamps; ++i){
								auto s = src[i];
								s = (s<-1.f ? -1.f : (s>1.f ? s=1.f : s)) * 32767.f;
								int16_t i16 = s<0.f ? s-0.5f : s+0.5f; // round to nearest int
								serializeToBigEndian(dst + i*2, i16);
							}
							break;
						}
						mSoundFile.write(mSoundFileSamples.data(), mSoundFileSamples.size());
					}
					else{
						//printf("SoundFile writer thread: overrun (sleeping...)\n");
						al_sleep(0.01);
					}
				}
			}
		);
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
			auto * glPBOs = (GLuint *)mPBOs;
			glDeleteBuffers(Npbos, glPBOs);
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

		mAudioIO->remove(mAudioCB);
	
		if(NON_REAL_TIME == mMode){
			mAudioIO->start();
		}

		mAudioIO = 0;
	}
}


void RenderToDisk::saveScreenshot(unsigned w, unsigned h, unsigned l, unsigned b){
	if(mDir.empty()) makeDir();
	saveImage(w,h,l,b, /*usePBO=*/false);
}

void RenderToDisk::saveScreenshot(al::Window& win){
	saveScreenshot(win.width(), win.height());
}


void RenderToDisk::makeDir(){
	// If no user dir specified, create default with time string
	if(mUserDir.empty()){
		mDir = "./render";
		mDir += al::timecodeNow("DHMS");
	}
	else{
		mDir = mUserDir;
	}

	// Create output directory if it doesn't exist
	if(!File::exists(mDir)) Dir::make(mDir);
}

void RenderToDisk::write(){
	writeImage();
	writeAudio();
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
	
	for(unsigned k=0; k<audioBlocks; ++k){

		// In non-real-time mode, this may write many blocks of audio. The read
		// thread must check for new samples often enough to avoid underruns.
		mAudioIO->processAudio();
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

	auto downloadPixels = [=](void * ptr){
		glPixelStorei(GL_PACK_ALIGNMENT, 1); // req'd for reading 3-component pixels
		glReadPixels(l,b,w,h, GL_RGB, GL_UNSIGNED_BYTE, ptr);
		glPixelStorei(GL_PACK_ALIGNMENT, 4); // back to default
	};

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
			auto * glPBOs = (GLuint *)mPBOs;
			glGenBuffers(Npbos, glPBOs);
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
			std::memcpy(pixs, ptr, numBytes);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			readPixels = true;
		}

		// This will perform asynchronously into the bound PBO
		downloadPixels(0);

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		mPBOIdx = (mPBOIdx + 1) % Npbos;
		mReadPBO = mReadPBO || (mPBOIdx == 0); // written to all PBOs at least once
	}
	else
	#endif
	{	// This will block until all draw commands finish
		downloadPixels(pixs);
		readPixels = true;
	}

	// Launch thread to write pixels out to an image file
	if(readPixels){
		//printf("----\nWriting frame %d\n", mFrameNumber);

		// At 40 FPS: 60 x 60 x 40 = 144000 frames/hour
		std::string name = mDir + "/" + al::toString("%07u", mFrameNumber) + "." + mImageExt;
		Image::Format format = Image::RGB;

		CHECK_THREADS:
		int i=0;
		for(; i<Nthreads; ++i){
			//printf("Checking if image writer %d is free ...\n", i);
			if(mImageWriters[i].run(name, mPixels, w,h, format, mImageCompress, mImagePaletteSize)){
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
			//al::Image::save(name, pixs, w,h, format, mImageCompress, mImagePaletteSize);
		}
	
		++mFrameNumber;
	}
}

RenderToDisk& RenderToDisk::ffmpegPath(const std::string& s){
	mFFMPEGDir = File::conformDirectory(s);
	return *this;
}

void RenderToDisk::createVideo(int videoCompress, int videoEncodeSpeed){

	// Nothing to do without image sequence
	if(!mWroteImages) return;

	std::string args;
	args += " -r " + al::toString(1./mFrameDur);
	args += " -i " + mDir + "/%07d." + mImageExt;
	if(mWroteAudio){
		args += " -i " + mDir + "/output.au -c:a aac -b:a 192k";
	}

	//args += " -pix_fmt yuv420p" // for compatibility with outdated media players
	//args += " -crf 20 -preset slower";
	args += " -crf " + al::toString(videoCompress);
	static const std::string speedStrings[] = {"placebo","veryslow","slower","slow","medium","fast","faster","veryfast","superfast","ultrafast"};
	args += " -preset " + speedStrings[videoEncodeSpeed];
	args += " " + mDir + "/movie.mp4";

	std::string cmd = "\"" + mFFMPEGDir + "ffmpeg" + "\"" + args;
	//printf("%s\n", cmd.c_str());

	// TODO: thread this; std::system blocks until the command finishes
	std::system(cmd.c_str());
}


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
	std::memcpy(&mBuffer[wblock * blockSamps], block, blockSamps * sizeof(mBuffer[0]));
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
	if(mInputInterleaved){
		for(unsigned i=0; i<mBlockSize; ++i){
			for(unsigned c=0; c<mChannels; ++c){
				dst[i*mChannels + c] = (*src++) * mGain;
			}
		}
	} else {
		for(unsigned c=0; c<mChannels; ++c){
			for(unsigned i=0; i<mBlockSize; ++i){
				dst[i*mChannels + c] = (*src++) * mGain;
			}
		}
	}

	// Big-endianize
	/*if(true){
		for(unsigned i=0; i<blockSizeInSamples(); ++i){
			float& s = dst[i];
			serializeToBigEndian(reinterpret_cast<char*>(&s), s);
		}
	}*/

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
	const std::string& dir,
	const std::vector<unsigned char>& pixels, unsigned w, unsigned h,
	Image::Format format, int compress, int paletteSize
){
	if(mBusy) return false;

	mBusy = true;

	// Create local copy of pixels
	mImage.resize<unsigned char>(w,h, format);
	std::memcpy(mImage.pixels<void>(), &pixels[0], pixels.size());

	mImage.compression(compress);
	mImage.paletteSize(paletteSize);
	mDir = dir;

	// Wait for any thread function in progress
	mThread.join();

	mThread.start(
		[this](){
			mImage.save(mDir);
			mBusy = false;
		}
	);

	return true;
}

}
