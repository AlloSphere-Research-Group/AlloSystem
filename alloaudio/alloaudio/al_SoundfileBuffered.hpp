#ifndef SOUNDFILEBUFFERED_H
#define SOUNDFILEBUFFERED_H

// Travis uses an older version of GCC which has a bug in its GCC version for
// clang so it fails to build when the mutex class is used.
// See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53841
#ifdef TRAVIS_BUILD
#define _GLIBCXX_USE_CLOCK_REALTIME
#endif

#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>

#include "Gamma/SoundFile.h"
#include "allocore/types/al_SingleRWRingBuffer.hpp"



namespace al
{

///
/// \brief Read a soundfile with buffering on a low priority thread
///
/// The SoundFileBuffered class is a wrapper around Gamma's SoundFile class.
/// It reads the soundfile in a separate thread and reading is done from a
/// lock-free ring buffer. This is the ideal way of reading a soundfile
/// within an audio callback as it will provide the most efficient mechanism
/// for low latency, high efficiency and drop-out free soundfile access.
///
///
class SoundFileBuffered
{
public:
	///
	/// \param fullPath The full path to the audio file
	/// \param loop set to true if you want the sound file to start over when finished
	/// \param bufferFrames the size of the ring buffer. Set to larger if experiencing dropouts or if planning to read more samples, e.g. the audio buffer size is large.
	///
	SoundFileBuffered(std::string fullPath, bool loop = false, int bufferFrames = 1024);
	~SoundFileBuffered();

	///
	/// \brief Read samples from the audio file
	/// \param buffer pre-allocated buffer of at least numFrames*channels() size
	/// \param numFrames number of frames to read
	/// \return the number of frames actually read.
	///
	int read(float *buffer, int numFrames);

	bool opened() const;								///< Returns whether the sound file is open
	gam::SoundFile::EncodingType encoding() const;	///< Get encoding type
	gam::SoundFile::Format format() const;			///< Get format
	double frameRate() const;							///< Get frames/second
	int frames() const;								///< Get number of frames
	int channels() const;								///< Get number of channels
	int samples() const;								///< Get number of samples ( = frames x channels)

	typedef void (*CallbackFunc)(float *buffer, int numChannels,
								 int numFrames, void * userData);

	///
	/// \brief Set a function that will be called whenever samples are read
	///
	/// The function func will be called whenever the low priority reader thread
	/// reads samples from the audio file. The callback function will get the
	/// samples that have just been read. This can be useful is the data is also
	/// required by another thread than the audio thread. For example if you want
	/// to display the audio data in addition to playing it. Bear in mind that if
	/// the process taking place in the callback function is too intensive it
	/// might produce an underrun in the ring buffer resulting in audio dropouts.
	/// If this is the case, use the callback to copy the data to a separate
	/// thread for processing.
	///
	/// \param func the callback function
	/// \param userData the data to be passed to the callback
	///
	void setReadCallback(CallbackFunc func, void *userData);

private:
	bool mRunning;
	bool mLoop;
	std::mutex mLock;
	std::condition_variable mCondVar;
	std::thread *mReaderThread;
	SingleRWRingBuffer *mRingBuffer;
	int mBufferFrames;

	gam::SoundFile mSf;
	CallbackFunc mReadCallback;
	void *mCallbackData;

	static void readFunction(SoundFileBuffered *obj);
};

} // namespace al

#endif // SOUNDFILEBUFFERED_H
