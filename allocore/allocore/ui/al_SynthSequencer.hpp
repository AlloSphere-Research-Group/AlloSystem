
#ifndef AL_SYNTHSEQUENCER_HPP
#define AL_SYNTHSEQUENCER_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2018. The Regents of the University of California.
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
	Event Voice and Sequencer

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <map>
#include <vector>
#include <list>
#include <limits.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <typeinfo> // For class name instrospection

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/io/al_AudioIOData.hpp"

//#include "Gamma/Domain.h"

namespace al
{

/**
 * @brief The SynthVoice class
 *
 * When inheriting this class you must provide a default construct that takes no arguments
*/
class SynthVoice {
public:

    /// Returns true if voice is currently active
    bool active() { return mActive;}

    /**
     * @brief Override this function to define audio processing.
     * @param io
     *
     * You will need to mark this instance as done by calling the
     *  free() function when envelopes or processing is done. You should
     * call free() from one of the render() functions. You can access the
     * note parameters using the getInstanceParameter(), getParameters()
     * and getOffParameters() functions.
     */
    virtual void onProcess(AudioIOData& io) {}

    /**
     * @brief Override this function to define graphics for this synth
     */
    virtual void onProcess(Graphics &g) {}

    /**
    * @brief  Override this function to determine what needs to be done when note/event starts.
    *
    * When a note starts, internal data within the algorithm usually needs to be reset,
    * e.g. reset envelopes, oscillator phase, etc.
    */
    virtual void onTriggerOn() {}

    /**
    * @brief  determine what needs to be done when note/event ends
    * Define this function to determine what needs to be done when note/event
    * ends. e.g. trigger release in envelopes, etc.
    * */
    virtual void onTriggerOff() {}

    /// This function can be called to programatically trigger  a voice.
    /// It is used for example in PolySynth to trigger a voice.
    void triggerOn(int offsetFrames = 0) {
        mOnOffsetFrames = offsetFrames;
        onTriggerOn();
        mActive = true;
    }

    /// This function can be called to programatically trigger the release
    /// of a voice.
    void triggerOff(int offsetFrames = 0) {
        mOffOffsetFrames = offsetFrames; // TODO implement offset frames for trigger off. Currently ignoring and turning off at start of buffer
        onTriggerOff();
    }

    /**
     * @brief returns the offset frames and sets them to 0.
     * @return offset frames
     *
     * Get the number of frames by which the start of this voice should be offset within a
     * processing block. This value is set to 0 once read as it should only
     * apply on the first rendering pass of a voice.
     */
    int getStartOffsetFrames() {
        int frames = mOnOffsetFrames;
        mOnOffsetFrames = 0;
        return frames;
    }

    int &getEndOffsetFrames() {return mOffOffsetFrames;}

protected:

    ///
    /**
     * @brief Mark this voice as done.
     *
     * This should be set within one of the render()
     * functions when envelope or time is done and no more processing for
     * the note is needed. The voice will be considered ready for retriggering
     * by PolySynth
     */
    void free() {mActive = false; } // Mark this voice as done.

private:
    int mActive {false};
    int mOnOffsetFrames {0};
    int mOffOffsetFrames {0};
};

class PolySynth {
public:
    PolySynth(unsigned int numPolyphony=64)
    {
        mVoices.reserve(numPolyphony);
    }

    /**
     * @brief trigger OnTriggers the start of a note event if a free note is available.
     * @param newVoice instance of the voice to trigger
     *
     * Currently no voice stealing is implemented. Note is only triggered if a voice
     * is free.
     *
     * You can use the id to identify the note for later triggerOff() calls
     */
    void triggerOn(std::shared_ptr<SynthVoice> voice) {
        auto allocedVoiceIter = mVoices.begin();
        for (; allocedVoiceIter != mVoices.end();allocedVoiceIter++) {
            std::shared_ptr<SynthVoice> allocedVoice = *allocedVoiceIter;
            if (!allocedVoice->active()) {
                *allocedVoiceIter = voice;
                break;
            }
        }
        if (allocedVoiceIter == mVoices.end()) {
            if (mVoices.size() >= mVoices.capacity()) {
                std::cout << "Allocating new voice. You might want to increase polyphony to " << mVoices.size() + 1 << std::endl;
            }
            mVoices.push_back(voice);
        }
        voice->triggerOn();
    }

    /// trigger release of voice with id
    void triggerOff(int id) {
        mVoices[id]->triggerOff();
    }

    /**
     * @brief render all the active voices into the audio buffers
     * @param io AudioIOData containing buffers and audio I/O meta data
     */
    void render(AudioIOData &io) {
        for (auto voice: mVoices) {
            if (voice->active()) {
                io.frame(voice->getStartOffsetFrames());
                int endOffsetFrames = voice->getEndOffsetFrames();
                if (endOffsetFrames >= 0) {
                    if (endOffsetFrames < io.framesPerBuffer()) {
                        voice->triggerOff(endOffsetFrames);
                    }
                    endOffsetFrames -= io.framesPerBuffer();
                }
                voice->onProcess(io);
            }
        }
    }

    /**
     * @brief render graphics for all active voices
     */
    void render(Graphics &g) {
        for (auto voice: mVoices) {
            if (voice->active()) {
                voice->onProcess(g);
            }
        }
    }

private:
    std::vector<std::shared_ptr<SynthVoice>> mVoices;  // External voices are added programatically
};

class SynthSequencerEvent {
public:
    double startTime {0};
    double duration {-1};
    int offsetCounter {0}; // To offset event within audio buffer
    std::shared_ptr<SynthVoice> voice;
};

/**
 * @brief Event Sequencer triggering audio visual "notes"
 *
 * Sequences can be created programatically:
 * @code
    SynthSequencer seq;
    seq.add<SineEnv>( 0  ).set(3.5, 260, 0.3, .011, .2);
    seq.add<SineEnv>( 0  ).set(3.5, 510, 0.3, .011, .2);
    seq.add<SineEnv>( 3.5).set(3.5, 233, 0.3, .011, .2);
    seq.add<SineEnv>( 3.5).set(3.5, 340, 0.3, .011, .2);
    seq.add<SineEnv>( 3.5).set(7.5, 710, 0.3, 1, 2);
 *  @endcode
 *
 * The render() functions need to be places within their relevant contexts like
 * the audio callback (e.g. onSound() ) or the graphics callback (e.g. onDraw())
 *
 * A time master can be selected in the constructor to define where the
 * sequencer runs. TIME_MASTER_AUDIO is more precise in time, but you might want
 * to use TIME_MASTER_GRAPHICS if your "note" produces no audio.
 *
 */

class SynthSequencer {
public:
    typedef enum {
        TIME_MASTER_AUDIO,
        TIME_MASTER_GRAPHICS
    } TimeMasterMode;

    SynthSequencer(unsigned int numPolyphony=64, TimeMasterMode masterMode = TIME_MASTER_AUDIO)
        : mPolySynth(numPolyphony), mMasterMode(masterMode)
    {
    }

    /// Insert this function within the audio callback
    void render(AudioIOData &io) {
        if (mMasterMode == TIME_MASTER_AUDIO) {
            double timeIncrement = io.framesPerBuffer()/(double) io.framesPerSecond();
            mMasterTime += timeIncrement;
            if (mNextEvent < mEvents.size()) {
                auto iter = mEvents.begin();
                std::advance(iter, mNextEvent);
                auto event = *iter;
                while (event.startTime <= mMasterTime) {
                    event.offsetCounter = 0.5f + (event.startTime - (mMasterTime - timeIncrement))*io.framesPerSecond();
                    mPolySynth.triggerOn(event.voice);
//                    std::cout << "Event " << mNextEvent << " " << event.startTime << " " << typeid(*event.voice.get()).name() << std::endl;

                    mNextEvent++;
                    iter++;
                    if (iter == mEvents.end()) {
                        break;
                    }
                    event = *iter;
                }
            }
        }
        mPolySynth.render(io);
    }

    /// Insert this function within the graphics callback
    void render(Graphics &g) {
        if (mMasterMode == TIME_MASTER_GRAPHICS) {
            double timeIncrement = 1.0/mFps;
            mMasterTime += timeIncrement;
            if (mNextEvent < mEvents.size()) {
                auto iter = mEvents.begin();
                std::advance(iter, mNextEvent);
                auto event = *iter;
                while (event.startTime <= mMasterTime) {
                    event.offsetCounter = 0;
                    mPolySynth.triggerOn(event.voice);
                    std::cout << "Event " << mNextEvent << " " << event.startTime << " " << typeid(*event.voice.get()).name() << std::endl;
                    mNextEvent++;
                    iter++;
                    if (iter == mEvents.end()) {
                        break;
                    }
                    event = *iter;
                }
            }
        }
        mPolySynth.render(g);
    }

    /// Set the frame rate at which the graphics run (i.e. how often render(Graphics &g)
    /// will be called
    void setGraphicsFrameRate(float fps) {mFps = fps;} // TODO this should be handled through Gamma Domains

    /**
     * @brief insert an event in the sequencer
     * @param startTime
     * @param duration
     * @return a reference to the voice instance inserted
     *
     * This function is not thread safe, so you must add all your notes before starting the
     * sequencer context (e.g. the audio callback if using TIME_MASTER_AUDIO). If you need
     * to insert events on the fly, use triggerOn() directly on the PolySynth member
     *
     * The TSynthVoice template must be a class inherited from SynthVoice.
     */
    template<class TSynthVoice>
    TSynthVoice &add(double startTime, double duration = -1) {
        // Insert into event list, sorted.
        auto position = mEvents.begin();
        while(position != mEvents.end() && position->startTime < startTime) {
            position++;
        }
        auto insertedEvent = mEvents.insert(position, SynthSequencerEvent());
        insertedEvent->startTime = startTime;
        insertedEvent->duration = duration;
        auto newVoice = std::make_shared<TSynthVoice>();
        insertedEvent->voice = newVoice;
        return *newVoice;
    }

    /**
     * @brief Basic audio callback for quick prototyping
     * @param io
     *
     * Pass this audio callback to an AudioIO object with a pointer to a
     *  SynthSequencer instance to hear the sequence.
     */
    static void audioCB(AudioIOData& io) {
        io.user<SynthSequencer>().render(io);
    }

private:
    PolySynth mPolySynth;

    double mFps {30}; // graphics frames per second

    unsigned int mNextEvent {0};
    std::list<SynthSequencerEvent> mEvents; // List of events sorted by start time.

    TimeMasterMode mMasterMode {TIME_MASTER_AUDIO};
    double mMasterTime {0};
};

}


#endif  // AL_SYNTHSEQUENCER_HPP
