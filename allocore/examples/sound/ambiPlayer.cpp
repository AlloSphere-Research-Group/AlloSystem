/*
What is our "Hello world!" app?

An agent orbits around the origin emitting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode 
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
#include <Gamma/SoundFile.h>
#include <iostream>

using namespace al;

#define AUDIO_BLOCK_SIZE 512

typedef struct {
    AmbisonicsSpatializer *spatializer;
    gam::SoundFile sf;
    float read_buffer[AUDIO_BLOCK_SIZE * 4];

} userdata_t;


void audioCB(AudioIOData& io)
{
    userdata_t * ud = (userdata_t *) io.user();
	int numFrames = io.framesPerBuffer();


    AmbisonicsSpatializer* spatializer = ud->spatializer;
    spatializer->prepare(io);

    float * ambiChans = spatializer->ambiChans();

    int framesRead = ud->sf.read(ud->read_buffer, numFrames);


    for (int i = 0; i < io.channelsOut(); i++) {
        for (int j = 0; j < framesRead; j++) {
            *ambiChans++ = ud->read_buffer[j*4 + i];
//            io.out(i, j) = ud->read_buffer[j*4 + i];
        }
    }
//    spatializer->perform(io,src,relpos, numFrames, i, s);


    spatializer->finalize(io);
}

#define STEREO

int main (int argc, char * argv[])
{
    // Set speaker layout
    AudioDevice::printAll();

#ifdef STEREO
    const int numSpeakers = 2;
    Speaker speakers[] = {
        Speaker(0,  45,0),
        Speaker(1, -45,0),
    };
#else
    const int numSpeakers = 30;
    Speaker speakers[numSpeakers] = {
        Speaker(1-1, 77.660913, 41.000000),
        Speaker(2-1, 45.088015, 41.000000),
        Speaker(3-1, 14.797289, 41.000000),
        Speaker(4-1, -14.797289, 41.000000),
        Speaker(5-1, -45.088015, 41.000000),
        Speaker(6-1, -77.660913, 41.000000),
        Speaker(7-1, -102.339087, 41.000000),
        Speaker(8-1, -134.911985, 41.000000),
        Speaker(9-1, -165.202711, 41.000000),
        Speaker(10-1, 165.202711, 41.000000),
        Speaker(11-1, 134.911985, 41.000000),
        Speaker(12-1, 102.339087, 41.000000),
        Speaker(17-1, 77.660913, 0.000000),
        Speaker(18-1, 65.647587, 0.000000),
        Speaker(19-1, 54.081600, 0.000000),
        Speaker(20-1, 42.869831, 0.000000),
        Speaker(21-1, 31.928167, 0.000000),
        Speaker(22-1, 21.181024, 0.000000),
        Speaker(23-1, 10.559657, 0.000000),
        Speaker(24-1, 0.000000, 0.000000),
        Speaker(25-1, -10.559657, 0.000000),
        Speaker(26-1, -21.181024, 0.000000),
        Speaker(27-1, -31.928167, 0.000000),
        Speaker(28-1, -42.869831, 0.000000),
        Speaker(29-1, -54.081600, 0.000000),
        Speaker(30-1, -65.647587, 0.000000),
        Speaker(31-1, -77.660913, 0.000000),
        Speaker(32-1, -102.339087, 0.000000),
        Speaker(33-1, -114.352413, 0.000000),
        Speaker(34-1, -125.918400, 0.000000),
        Speaker(35-1, -137.130169, 0.000000),
        Speaker(36-1, -148.071833, 0.000000),
        Speaker(37-1, -158.818976, 0.000000),
        Speaker(38-1, -169.440343, 0.000000),
        Speaker(39-1, -180.000000, 0.000000),
        Speaker(40-1, 169.440343, 0.000000),
        Speaker(41-1, 158.818976, 0.000000),
        Speaker(42-1, 148.071833, 0.000000),
        Speaker(43-1, 137.130169, 0.000000),
        Speaker(44-1, 125.918400, 0.000000),
        Speaker(45-1, 114.352413, 0.000000),
        Speaker(46-1, 102.339087, 0.000000),
        Speaker(49-1, 77.660913, -32.500000),
        Speaker(50-1, 45.088015, -32.500000),
        Speaker(51-1, 14.797289, -32.500000),
        Speaker(52-1, -14.797289, -32.500000),
        Speaker(53-1, -45.088015, -32.500000),
        Speaker(54-1, -77.660913, -32.500000),
        Speaker(55-1, -102.339087, -32.500000),
        Speaker(56-1, -134.911985, -32.500000),
        Speaker(57-1, -165.202711, -32.500000),
        Speaker(58-1, 165.202711, -32.500000),
        Speaker(59-1, 134.911985, -32.500000),
        Speaker(60-1, 102.339087, -32.500000),
    };
#endif

    SpeakerLayout speakerLayout;
    for (int i = 0; i< numSpeakers; i++) {
        speakerLayout.addSpeaker(speakers[i]);
    }

    userdata_t ud;
    // Create spatializer
    ud.spatializer = new AmbisonicsSpatializer(speakerLayout, 2, 1);
    ud.spatializer->numSpeakers(numSpeakers);
    ud.spatializer->numFrames(AUDIO_BLOCK_SIZE);

    std::string filename = "/home/andres/Music/Ambisonics/Ambisonia/AF_Orfeo_Barocco_REC2.amb";
//    std::cout << ud.spatializer->numChannels() << std::endl;
    ud.sf.path(filename);
    if (!ud.sf.openRead()) {
        std::cout << " Can't open file " << std::endl;
        return -1;
    } else {
        std::cout << "Playing file: " << filename << std::endl;
    }
//    Dbap *dbap;
//    dbap = new Dbap();

    // Create listener to render audio
//	listener = scene.createListener(speakerLayout, ambisonics);
    //listener = scene.createListener(speakerLayout, dbap);
	

    AudioIO audioIO(AUDIO_BLOCK_SIZE, 44100, audioCB, &ud, numSpeakers, numSpeakers);
    audioIO.deviceIn(6);
    audioIO.deviceOut(7);
	audioIO.start();

	MainLoop::start();
	return 0;
}
