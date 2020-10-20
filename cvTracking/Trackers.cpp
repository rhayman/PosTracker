#include "cvTrackers.hpp"
#include "Trackers.hpp"
#include "TrackersEditor.hpp"

Trackers::Trackers() : GenericProcessor("Tracker API")
{
	setProcessorType (PROCESSOR_TYPE_FILTER);
	sendSampleCount = false;
}

Trackers::~Trackers() {}

void Trackers::updateSettings()
{
	if ( editor != NULL )
		editor->updateSettings();
}

void Trackers::handleEvent(const EventChannel * eventInfo, const MidiMessage & event, int samplePosition) {
	// std::cout << "eventInfo->getName() " << eventInfo->getName() << std::endl;
}

AudioProcessorEditor * Trackers::createEditor() {
	editor = new TrackersEditor(this, true);
	return editor;
}

void Trackers::saveCustomParametersToXml(XmlElement * xml)
{
	xml->setAttribute("Type", "Trackers");
	XmlElement * paramXml = xml->createNewChildElement("Tracker");
	auto ed = static_cast<TrackersEditor*>(getEditor());
	auto name = ed->getTrackerName();
	paramXml->setAttribute("TrackerName", name);
	ed->saveToXml(paramXml);
}

void Trackers::loadCustomParametersFromXml()
{
	auto ed = static_cast<TrackersEditor*>(getEditor());
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Tracker")
		ed->loadXmlParams(paramXml);
	updateSettings();
};
