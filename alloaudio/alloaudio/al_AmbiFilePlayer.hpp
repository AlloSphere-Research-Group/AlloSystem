#ifndef AL_AMBIFILEPLAYER_H
#define AL_AMBIFILEPLAYER_H

#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "allocore/io/al_AudioIOData.hpp"
#include "allocore/ui/al_Parameter.hpp"
#include "alloaudio/al_SoundfileBuffered.hpp"
#include "alloaudio/al_AmbiTunedDecoder.hpp"
#include "alloaudio/al_AmbisonicsConfig.hpp"

namespace al {

using namespace std;

/// \addtogroup alloaudio
///  @{

///
/// \brief A class to play back Ambisonics encoded (B-format) audio files
///
class AmbiFilePlayer : public AudioCallback, public SoundFileBuffered
{

public:
	///
	/// \brief AmbiFilePlayer constructor
	/// \param fullPath full path to the b-format audio file
	/// \param loop true if file should start over when it reaches the end
	/// \param bufferFrames Number of frames in the audio file read buffer. Increase if experiencing dropouts
	/// \param layout the speaker layout for decoding
	///
	AmbiFilePlayer(std::string fullPath, bool loop, int bufferFrames,
	               SpeakerLayout &layout);
	///
	/// \param fullPath full path to the b-format audio file
	/// \param loop true if file should start over when it reaches the end
	/// \param bufferFrames Number of frames in the audio file read buffer. Increase if experiencing dropouts
	/// \param configPath path to the .ambdec ambisonics configuration file. If empty the file "Ambisonics.ambdec" is used.
	///
	AmbiFilePlayer(std::string fullPath, bool loop, int bufferFrames,
	               string configPath);

	~AmbiFilePlayer();

	virtual void onAudioCB(AudioIOData& io) /*override*/;

	///
	/// \brief Check whether file has been played fully
	/// \return true if file has played to the end
	///
	bool done() const;
	void setDone(bool done);

private:

	int getFileDimensions();
	int getFileOrder();

	// Internal

	AmbiDecode *mDecoder;
	float *mReadBuffer;
	float *mDeinterleavedBuffer;
	bool mDone;
	int mBufferSize;

	//Parameters
	Parameter mGain;

};

/// @}
}

#endif // AL_AMBIFILEPLAYER_H
