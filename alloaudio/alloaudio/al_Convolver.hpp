#ifndef AL_CONVOLVER_H
#define AL_CONVOLVER_H

#include <vector>
#include "allocore/io/al_AudioIO.hpp"

#define MAXSIZE 0x00100000
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


class Convolver
{

public:
    Convolver();

	/**
	 * @brief configure
	 *
	 * Output from disabled channels is set to 0.
	 *
	 * @param io
	 * @param IRs
	 * @param inputs
	 * @param inputsAreBuses
	 * @param disabledChannels
	 * @param maxsize
	 * @param minpartition
	 * @param maxpartition
	 * @return
	 */
	int configure(al::AudioIO &io,
				  vector<float *>IRs,
                  vector<int> IRlengths,
				  int inputChannel = -1,
				  bool inputsAreBuses = false,
				  vector<int> disabledChannels = vector<int>(),
				  unsigned int basePartitionSize=64, unsigned int options=0);
	/**
	 * @brief processBlock
	 * @param io
	 * @return
	 */
	int processBlock(AudioIO &io);
    
    int shutdown(void);

private:
	vector<int> m_activeChannels;
	vector<int> m_disabledChannels;
	int m_inputChannel;
	bool m_inputsAreBuses;
	Convproc *m_Convproc;
};

}

#endif // AL_CONVOLVER_H
