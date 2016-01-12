#include "alloaudio/al_SoundfileBuffered.hpp"

using namespace al;

SoundFileBuffered::SoundFileBuffered(std::string fullPath, bool loop, int bufferFrames) :
    mRunning(true),
    mLoop(loop),
    mRepeats(0),
    mBufferFrames(bufferFrames),
    mReadCallback(0)
{
	mSf.path(fullPath);
	mSf.openRead();
	if (mSf.opened()) {
		mRingBuffer = new SingleRWRingBuffer(mBufferFrames * channels() * sizeof(float));
		mReaderThread = new std::thread(readFunction, this);
	}
}

SoundFileBuffered::~SoundFileBuffered()
{
	if (mSf.opened()) {
		mRunning = false;
		mCondVar.notify_one();
		mReaderThread->join();
		delete mReaderThread;
		delete mRingBuffer;
	}
	mSf.close();
}

int SoundFileBuffered::read(float *buffer, int numFrames)
{
	int bytesRead = mRingBuffer->read((char *) buffer, numFrames * channels() * sizeof(float));
	if (bytesRead != numFrames * channels() * sizeof(float)) {
		// TODO: handle underrun
	}
	mCondVar.notify_one();
	return bytesRead / (channels() * sizeof(float));
}

bool SoundFileBuffered::opened() const
{
	return mSf.opened();
}

void SoundFileBuffered::readFunction(SoundFileBuffered  *obj)
{
	float buf[obj->mBufferFrames * obj->channels()];
	while (obj->mRunning) {
		std::unique_lock<std::mutex> lk(obj->mLock);
		obj->mCondVar.wait(lk);
		int framesToRead = obj->mRingBuffer->writeSpace() / (obj->channels() * sizeof(float));
		int framesRead = obj->mSf.read(buf, framesToRead);
		if (framesRead != framesToRead && obj->mLoop) {
			obj->mSf.seek(0, SEEK_SET);
			int framesRead = obj->mSf.read(buf + framesRead, framesToRead - framesRead);
			if (framesRead == 0) {
				// TODO: Handle IO error
			}
			std::atomic_fetch_add(&(obj->mRepeats), 1);
		}
		int written = obj->mRingBuffer->write((const char*) buf, framesRead * sizeof(float) * obj->channels());
//		if (written != framesRead * sizeof(float) * obj->channels()) {
//			// TODO handle overrun
//		}
		if (obj->mReadCallback) {
			obj->mReadCallback(buf, obj->mSf.channels(), framesRead, obj->mCallbackData);
		}
		lk.unlock();
	}
}

gam::SoundFile::EncodingType al::SoundFileBuffered::encoding() const
{
	if (opened()) {
		return mSf.encoding();
	} else {
		return (gam::SoundFile::EncodingType) 0;
	}
}

gam::SoundFile::Format SoundFileBuffered::format() const
{
	if (opened()) {
		return mSf.format();
	} else {
		return (gam::SoundFile::Format) 0;
	}
}

double SoundFileBuffered::frameRate() const
{
	if (opened()) {
		return mSf.frameRate();
	} else {
		return 0.0;
	}
}

int SoundFileBuffered::frames() const
{
	if (opened()) {
		return mSf.frames();
	} else {
		return 0;
	}
}

int SoundFileBuffered::channels() const
{
	if (opened()) {
		return mSf.channels();
	} else {
		return 0;
	}
}

int SoundFileBuffered::samples() const
{
	if (opened()) {
		return mSf.samples();
	} else {
		return 0;
	}
}

int al::SoundFileBuffered::repeats()
{
	return mRepeats.load();
}

void SoundFileBuffered::setReadCallback(SoundFileBuffered::CallbackFunc func, void *userData)
{
	mReadCallback = func;
	mCallbackData = userData;
}
