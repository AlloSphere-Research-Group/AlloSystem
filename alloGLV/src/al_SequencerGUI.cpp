
#include "alloGLV/al_SequencerGUI.hpp"

using namespace al;


glv::View *SequencerGUI::makePresetHandlerView(PresetHandler &handler, float buttonScale)
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

glv::View *SequencerGUI::makeSequencerPlayerView(PresetSequencer &sequencer)
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

glv::View *SequencerGUI::makeSequencerView(PresetSequencer &sequencer)
{
	// TODO implement
	return new glv::View; // For now to avoid compiler warnings.
}

glv::View *SequencerGUI::makeRecorderView(SequenceRecorder &recorder)
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

glv::View *SequencerGUI::makePresetMapperView(PresetMapper &presetMapper, bool showArchives)
{
	glv::DropDown *mapperSelector = new glv::DropDown;
	std::vector<std::string> mapList = presetMapper.listAvailableMaps(showArchives);

	for(std::string map: mapList) {
		mapperSelector->addItem(map);
	}

	mapperSelector->attach([](const glv::Notification& n) {
		glv::DropDown &dd = *n.sender<glv::DropDown>();
		PresetMapper &mapper = *n.receiver<PresetMapper>();
		std::cout << dd.getValue() << std::endl;
		mapper.load(dd.getValue());
	}, glv::Update::Value, &presetMapper);

	glv::Table *layout = new glv::Table("><");
	mapperSelector->width(120);
	*layout << mapperSelector << new glv::Label("Map");
//	        << startSequence << new glv::Label("Start sequence");

	layout->arrange();
	return layout;
}
