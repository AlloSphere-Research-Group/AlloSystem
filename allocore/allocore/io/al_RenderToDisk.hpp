#ifndef INCLUDE_AL_RENDER_TO_DISK_HPP
#define INCLUDE_AL_RENDER_TO_DISK_HPP
/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Utility to render sound/graphics to disk.

	File author(s):
	Lance Putnam, 2015, putnam.lance@gmail.com
*/

#include <fstream>
#include <string>
#include <vector>
#include "allocore/system/al_Thread.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/io/al_Window.hpp"

namespace al{

/// Renders sound and/or graphics to disk
///
/// @ingroup allocore
class RenderToDisk : public WindowEventHandler{
public:

	/// Rendering modes
	enum Mode{
		NON_REAL_TIME,	/**< Non-real-time rendering */
		REAL_TIME		/**< Real-time rendering */
	};

	enum SoundFormat{
		FLOAT32,
		PCM16
	};

	using AudioChans = std::vector<unsigned short>;


	/// @param[in] mode		rendering mode, /see mode
	RenderToDisk(Mode mode = REAL_TIME);

	~RenderToDisk();


	/// Returns whether rendering is taking place
	bool active() const { return mActive; }

	/// Get rendering mode
	Mode mode() const { return mMode; }

	/// Get directory to render files
	const std::string& dir() const { return mDir; }


	/// Adapts frame duration used in model/animation updates

	/// This will set the passed in argument to the rendering frame duration
	/// if rendering is active. This should be called if the render mode is
	/// NON_REAL_TIME and the application's animation depends on a delta time.
	template <class T>
	void adaptFrameDur(T& realtimeFrameDur) const {
		if(mActive && mWindow) realtimeFrameDur = mFrameDur;
	}

	/// Set directory for output files
	RenderToDisk& dir(const std::string& v);

	/// Set rendering mode (only when not rendering)

	/// This specifies whether rendering should occur in real-time or
	/// non-real-time. If REAL_TIME, then graphics and audio run at their
	/// current rates and buffering to disk happens as fast as possible. The
	/// audio and graphics settings will determine whether the rendering can
	/// keep up with real-time. If NON_REAL_TIME, then the graphics and audio
	/// i/o are taken over and the entire rendering process occurs as fast as 
	/// possible.
	RenderToDisk& mode(Mode v);

	/// Set format of image files
	RenderToDisk& imageFormat(const std::string& ext, int compression=50, int paletteSize=-1);

	/// Set format of sound file
	RenderToDisk& soundFormat(SoundFormat fmt);

	/// Set post-gain of captured audio
	RenderToDisk& audioGain(float v);

	AudioChans& audioChans(){ return mAudioChans; }

	/// Start rendering

	/// The soundfile sample rate and number of channels will be taken directly
	/// from the audio i/o parameters.
	/// @param[in] aio		the audio i/o to use
	/// @param[in] win		the window to use
	/// @param[in] fps		the graphics frame rate; if -1, use the window frame rate
	///
	/// \returns true on success
	bool start(al::AudioIO& aio, al::Window& win, double fps=-1);
	bool start(al::AudioIO& aio);
	bool start(al::Window& win, double fps=-1);

	/// Stop rendering
	void stop();

	/// Toggle rendering
	bool toggle(al::AudioIO& aio, al::Window& win, double fps=-1);
	bool toggle(al::AudioIO& aio);
	bool toggle(al::Window& win, double fps=-1);

	/// Save a screenshot to disk

	/// This saves an image of the currently active frame buffer to disk.
	/// @param[in] w	width in pixels
	/// @param[in] h	height in pixels
	/// @param[in] l	column of left pixel
	/// @param[in] b	row of bottom pixel
	void saveScreenshot(unsigned w, unsigned h, unsigned l=0, unsigned b=0);

	/// Save a screenshot of a window to disk
	void saveScreenshot(al::Window& win);

	/// Set path to FFMPEG (used for video creation)

	/// Usually this does not need to be set as ffmpeg will be in the system
	/// path. You can test by opening a terminal and simply running "ffmpeg".
	/// Otherwise, on Windows it might be "c:\\Program Files\\ffmpeg\\bin\\".
	RenderToDisk& ffmpegPath(const std::string& path);

	/// Create a video from last captured image frames and audio
	///
	/// @param[in] videoCompress		video compression amount in [0,51];
	///									inversely related to quality
	/// @param[in] videoEncodeSpeed		video encode speed amount in [0,9];
	///									inversely related to file size
	void createVideo(int videoCompress=23, int videoEncodeSpeed=4);

private:

	struct AudioRing{
		std::vector<float> mBuffer;
		unsigned mChannels, mBlockSize, mNumBlocks;
		unsigned mWriteBlock=0, mReadBlock=0;
		float mGain = 1.f;
		bool mInputInterleaved=true;

		void resize(unsigned channels, unsigned blockSize, unsigned numBlocks);
		void write(const float * block);
		int read();
		const float * readBuffer() const;
		unsigned blockSizeInSamples() const;
	};

	struct ImageWriter{
		Thread mThread;
		Image mImage;
		std::string mDir;
		bool mBusy = false;

		bool run(
			const std::string& path,
			const std::vector<unsigned char>& pixels, unsigned w, unsigned h,
			Image::Format format, int compress, int paletteSize
		);
	};

	Mode mMode;
	std::string mUserDir, mDir, mFFMPEGDir;
	unsigned mFrameNumber = 0;
	double mElapsedSec = 0.;

	al::Window * mWindow = NULL;
	double mFrameDur; // graphics frame duration
	double mWindowFPS;
	std::vector<unsigned char> mPixels;
	int mGraphicsBuf = -1;

	enum { Npbos = 3 };
	int mPBOs[Npbos];
	int mPBOIdx;
	bool mReadPBO;

	enum { Nthreads = 8 };
	ImageWriter mImageWriters[Nthreads];
	std::string mImageExt = "png";
	int mImageCompress = 50;
	int mImagePaletteSize = -1;

	al::AudioIO * mAudioIO = NULL;
	al::AudioIO::Callback mAudioCB;
	AudioChans mAudioChans; // channels to capture
	AudioRing mAudioRing;
	std::ofstream mSoundFile;
	SoundFormat mSoundFormat = FLOAT32;
	std::vector<char> mSoundFileSamples;
	Thread mSoundFileThread;

	bool mActive = false;
	bool mWroteImages = false, mWroteAudio = false;

	virtual bool onFrame();
	void makeDir();
	bool start(al::AudioIO * aio, al::Window * win, double fps=-1);
	bool toggle(al::AudioIO * aio, al::Window * win, double fps=-1);
	void write(); // Write next block of audio and current frame buffer to files
	void writeAudio(); // Write next block of audio to sound file
	void writeImage(); // Write current frame buffer to image file
	void resetPBOQueue();
	void saveImage(unsigned w, unsigned h, unsigned l=0, unsigned b=0, bool usePBO=true);
};

} // al::
#endif


