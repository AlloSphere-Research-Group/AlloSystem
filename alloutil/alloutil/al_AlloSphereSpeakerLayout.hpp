#ifndef AL_ALLOSPHERE_SPEAKER_LAYOUT_H
#define AL_ALLOSPHERE_SPEAKER_LAYOUT_H

#include "allocore/sound/al_Speaker.hpp"

#define NUM_SPEAKERS 54

namespace al {

/// Current arrangement of speakers in AlloSphere
struct AlloSphereSpeakerLayout : public SpeakerLayout{
   AlloSphereSpeakerLayout(){
    Speaker alloSpeakers[] = {
      Speaker(1-1, 102.339087, 41.000000),
      Speaker(2-1, 65.479774, 41.000000),
      Speaker(3-1, 22.878659, 41.000000),
      Speaker(4-1, -22.878659, 41.000000),
      Speaker(5-1, -65.479774, 41.000000),
      Speaker(6-1, -102.339087, 41.000000),
      Speaker(7-1, -77.660913, 41.000000),
      Speaker(8-1, -114.520226, 41.000000),
      Speaker(9-1, -157.121341, 41.000000),
      Speaker(10-1, 157.121341, 41.000000),
      Speaker(11-1, 114.520226, 41.000000),
      Speaker(12-1, 77.660913, 41.000000),
      Speaker(17-1, 102.339087, 0.000000),
      Speaker(18-1, 89.778386, 0.000000),
      Speaker(19-1, 76.570355, 0.000000),
      Speaker(20-1, 62.630227, 0.000000),
      Speaker(21-1, 47.914411, 0.000000),
      Speaker(22-1, 32.455914, 0.000000),
      Speaker(23-1, 16.397606, 0.000000),
      Speaker(24-1, 0.000000, 0.000000),
      Speaker(25-1, -16.397606, 0.000000),
      Speaker(26-1, -32.455914, 0.000000),
      Speaker(27-1, -47.914411, 0.000000),
      Speaker(28-1, -62.630227, 0.000000),
      Speaker(29-1, -76.570355, 0.000000),
      Speaker(30-1, -89.778386, 0.000000),
      Speaker(31-1, -102.339087, 0.000000),
      Speaker(32-1, -77.660913, 0.000000),
      Speaker(33-1, -90.221614, 0.000000),
      Speaker(34-1, -103.429645, 0.000000),
      Speaker(35-1, -117.369773, 0.000000),
      Speaker(36-1, -132.085589, 0.000000),
      Speaker(37-1, -147.544086, 0.000000),
      Speaker(38-1, -163.602394, 0.000000),
      Speaker(39-1, -180.000000, 0.000000),
      Speaker(40-1, 163.602394, 0.000000),
      Speaker(41-1, 147.544086, 0.000000),
      Speaker(42-1, 132.085589, 0.000000),
      Speaker(43-1, 117.369773, 0.000000),
      Speaker(44-1, 103.429645, 0.000000),
      Speaker(45-1, 90.221614, 0.000000),
      Speaker(46-1, 77.660913, 0.000000),
      Speaker(49-1, 102.339087, -32.500000),
      Speaker(50-1, 65.479774, -32.500000),
      Speaker(51-1, 22.878659, -32.500000),
      Speaker(52-1, -22.878659, -32.500000),
      Speaker(53-1, -65.479774, -32.500000),
      Speaker(54-1, -102.339087, -32.500000),
      Speaker(55-1, -77.660913, -32.500000),
      Speaker(56-1, -114.520226, -32.500000),
      Speaker(57-1, -157.121341, -32.500000),
      Speaker(58-1, 157.121341, -32.500000),
      Speaker(59-1, 114.520226, -32.500000),
      Speaker(60-1, 77.660913, -32.500000),
    };
    mSpeakers.reserve(NUM_SPEAKERS);
    for(int i=0; i<NUM_SPEAKERS; ++i)
      addSpeaker(alloSpeakers[i]);
  }
};

}  // al
#endif
