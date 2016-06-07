#ifndef AL_SEQUENCERGUI_HPP
#define AL_SEQUENCERGUI_HPP
/*	AlloSystem --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2016. The Regents of the University of California.
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
	GUI generation for sequencer

	File author(s):
	Andres Cabrera 2016, andres@mat.ucsb.edu
*/

#include "allocore/ui/al_SequenceRecorder.hpp"
#include "allocore/ui/al_PresetSequencer.hpp"
#include "alloGLV/al_ParameterGUI.hpp"
#include "GLV/glv.h"

namespace al {

class SequencerGUI
{
public:

	static glv::View *makePresetHandlerView(PresetHandler &handler, float buttonScale = 1.0f)
	{
		glv::Box *box = new glv::Box(glv::Direction::S);
		PresetButtons *presetButtons = new PresetButtons(glv::Rect(), 10, 4, false, true);

		PresetViewData *viewData = new PresetViewData;
		viewData->presetButtons= presetButtons;
		viewData->presetHandler = &handler;
		handler.registerPresetCallback(SequencerGUI::presetChangeCallback, (void *) viewData);
		presetButtons->enable(glv::Property::SelectOnDrag);
		presetButtons->width(240* buttonScale);
		presetButtons->height(96* buttonScale);
		*box << new glv::Label("Presets");

		glv::Box *nameBox = new glv::Box(glv::Direction::E);
		glv::TextView *presetNameView = new glv::TextView;
		*nameBox << new glv::Label("Name") << presetNameView;
		*box << nameBox;
		*box << presetButtons;
		glv::Box *storeBox = new glv::Box(glv::Direction::E);
		glv::Button *storeButton = new glv::Button(glv::Rect(150* buttonScale, 24* buttonScale));
		*storeBox << new glv::Label("Store", glv::Place::CL, 0, 0) << storeButton;
		*box << storeBox;

		glv::Box *morphBox = new glv::Box(glv::Direction::E);
		glv::NumberDialer *morphTime = new glv::NumberDialer(2,2, 20, 0);
		*morphBox << new glv::Label("Morph time") << morphTime;
		*box << morphBox;

		// Now register callbacks for morph button
		morphTime->attach([](const glv::Notification &n) {
			glv::Widget &sender = *n.sender<glv::Widget>();
			static_cast<PresetHandler *>(n.receiver())->setMorphTime(sender.getValue<double>());
			std::cout << sender.getValue<double>() << std::endl;
		},
		glv::Update::Value, &handler);

		handler.registerMorphTimeCallback([](float value, void *sender,
		                                  void *userData, void * blockSender) {
			static_cast<glv::NumberDialer *>(userData)->setValue(value);
		}, morphTime);

		//	WidgetWrapper *wrapper = new WidgetWrapper;
		//	wrapper->parameter = &mMorphTime;
		//	wrapper->lock = &mParameterGUILock;
		//	wrapper->widget = static_cast<glv::Widget *>(morphTime);
		//	mWrappers.push_back(wrapper);

		//	mMorphTime.registerChangeCallback([](float value, void *sender,
		//	                                  void *userData, void * blockSender) {
		//		valueChangedCallback(value, sender, userData, blockSender);
		//	}, wrapper);

		// Register callbacks for preset buttons
		presetButtons->attach(SequencerGUI::presetSavedInButton,
		                      glv::Update::Value, (void *) storeButton);

		presetButtons->setPresetHandler(handler);

		storeButton->attachVariable(&presetButtons->mStore, 1);
		presetNameView->attachVariable(&presetButtons->presetName, 1);
		box->fit();
		return box;
	}

	static glv::View *makeSequencerPlayerView(PresetSequencer &sequencer)
	{
		glv::Button *startSequence = new glv::Button(glv::Rect(60,20));
		glv::DropDown *sequenceSelector = new glv::DropDown;
		std::vector<std::string> sequenceList = sequencer.getSequenceList();

		for(std::string sequenceName: sequenceList) {
			sequenceSelector->addItem(sequenceName);
		}

		glv::Table *layout = new glv::Table("><");
		sequenceSelector->width(60);
		*layout << sequenceSelector << new glv::Label("Sequence")
		       << startSequence << new glv::Label("Start sequence");

		SequencePlayerData *playerData = new SequencePlayerData;
		playerData->sequencer = &sequencer;
		playerData->sequenceSelector = sequenceSelector;

		startSequence->attach([](const glv::Notification& n) {
			glv::Button& b = *n.sender<glv::Button>();
			SequencePlayerData *playerData = n.receiver<SequencePlayerData>();
			if (b.getValue()) {
				if (playerData->sequencer->running()) { playerData->sequencer->stopSequence();}
				playerData->sequencer->playSequence(playerData->sequenceSelector->getValue());
			} else {
				if (playerData->sequencer->running()) { playerData->sequencer->stopSequence(); }
				else {
					b.setValue(true);
					playerData->sequencer->playSequence(playerData->sequenceSelector->getValue());
				}
			}
		}, glv::Update::Value, playerData);
		layout->arrange();
		return layout;
	}

	static glv::View *makeSequencerView(PresetSequencer &sequencer)
	{

	}

	static glv::View *makeRecorderView(SequenceRecorder &recorder)
	{
		glv::Table *layout = new glv::Table("><");
		layout->enable(glv::DrawBack);
		glv::TextView *recordNameView = new glv::TextView(glv::Rect(60,20));
		recordNameView->setValue("new_seq");
		glv::Button *recordButton = new glv::Button(glv::Rect(60,20));
		RecordViewData *recordData = new RecordViewData;
		recordData->recorder = &recorder;
		recordData->recordNameTextView = recordNameView;
		recordButton->attach([](const glv::Notification& n) {
			glv::Button &b = *n.sender<glv::Button>();
			RecordViewData *recordData = n.receiver<RecordViewData>();
			if (b.getValue()) {
				recordData->recorder->startRecord(recordData->recordNameTextView->getValue());
			} else {
				recordData->recorder->stopRecord();
			}
		}, glv::Update::Value, recordData);
		*layout << recordNameView << new glv::Label("Record Name");
		*layout << recordButton << new glv::Label("Record");
		layout->arrange();
		//	layout->anchor(glv::Place::TR);
		//	layout->set(-200, 0, layout.width(), layout.height());
		return layout;
	}
private:
	struct PresetViewData {
		PresetButtons *presetButtons;
		PresetHandler *presetHandler;
	};

	struct SequencePlayerData {
		PresetSequencer *sequencer;
		glv::DropDown *sequenceSelector;
	};

	struct RecordViewData {
		SequenceRecorder *recorder;
		glv::TextView *recordNameTextView;
	};

	static void presetChangeCallback(int index, void *sender,void *userData)
	{
		PresetViewData *viewData = static_cast<PresetViewData *>(userData);
		glv::Data &d = viewData->presetButtons->data();
		const int *shape = d.shape();
		for (int x = 0; x < shape[0]; ++x) {
			for (int y = 0; y < shape[1]; ++y) {
				d.assign<int>(false, x, y); // Must manually set them all off...
			}
		}
		viewData->presetButtons->presetName = viewData->presetHandler->availablePresets()[index];
		d.assign<int>(true, index); // Do it this way to avoid the GUI triggering callbacks.

		//		std::cout << "preset Change callback" << std::endl;
	}

	static void presetSavedInButton(const glv::Notification &n) {
		glv::Button *button = static_cast<glv::Button *>(n.receiver());
		button->setValue(false);
	}
};

}

#endif // AL_SEQUENCERGUI_HPP
