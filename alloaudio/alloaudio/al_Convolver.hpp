#ifndef AL_CONVOLVER_H
#define AL_CONVOLVER_H

#include <vector>
#include "allocore/io/al_AudioIO.hpp"

#define MAXSIZE 0x00100000

/** @defgroup alloaudio AlloAudio */


class Convproc;

namespace al {

using namespace std;

// Number of IRs must always match the number of output channels
// Use cases:
// - Decorrelation: many to many each with a different IR
// - Auralization: one to many each with a different IR
//               : many to many each with a different IR
// - DRC : many to many each with different IR
// - Synthesized source:

    /**
     * @brief Convolver Realtime multichannel convolution class.
	 * @ingroup alloaudio
     *
     * Built on zita convolver, which implements a realtime multithreaded multichannel convolution algorithm using non-uniform partitioning.
     *
	 */
class Convolver : public al::AudioCallback
{

public:
    Convolver();

	/**
	 * @brief Sets up convolver. Must be called prior to processing.
	 *
	 * Checks for valid parameters, initializes convolver and impulse responses. Output from disabled channels is set to 0.
	 *
	 * @param[in, out] io The AudioIO object.
	 * @param[in] IRs The deinterleaved IR channels.
	 * @param[in] inputChannel Specifies input channel for one to many mode, otherwise set to -1 for many to many.
	 * @param[in] inputsAreBuses Set to True if you wish to use AudioIO's busses as input. This must be specified on AudioIO by calling io.channelsBus as well.
	 * @param[in] disabledChannels Contains list of all channels which should not be processed.
	 * @param[in] basePartitionSize Should be set to audio callback size to minimize latency. Cannot be less than 64 samples.
	 * @param[in] options Options to be passed to zita convolver. Currently supports OPT_FFTW_MEASURE = 1,
		OPT_VECTOR_MODE  = 2.
	 * @return Returns 0 upon success
	 */
	int configure(al::AudioIO &io,
				  vector<float *>IRs,
                  int IRlength,
				  int inputChannel = -1,
				  bool inputsAreBuses = false,
				  vector<int> disabledChannels = vector<int>(),
				  unsigned int basePartitionSize=64, unsigned int options=0);
	/**
	 * @brief Handles all io for the convolution
	 * @param[in,out] io The AudioIO object from which audio data will be read from and written to.
	 */
	virtual void onAudioCB(AudioIOData &io);
    
    /**
     * @brief Stops processing audio and tears down convolver object.
     * @return Returns 0 upon success.
     */
    int shutdown(void);

private:
	vector<int> m_activeChannels;
	vector<int> m_disabledChannels;
	int m_inputChannel;
	bool m_inputsAreBuses;
	Convproc *m_Convproc;
};

/** @} */
}

#endif // AL_CONVOLVER_H
