#ifndef TRACKERS_H_
#define TRACKERS_H_

#include <vector>
#include <ProcessorHeaders.h>
#include "../common.h"

class Trackers : public GenericProcessor
{
public:
	Trackers();
	~Trackers();
	bool isSource() const override { return false; }
	bool isSink() const override { return false; }
	bool isFilter() const override { return true; }
	bool hasEditor() const override { return true; }

	bool isReady() override { return true; }
	void process(AudioSampleBuffer& buffer) override { checkForEvents(); }
	void handleEvent(const EventChannel *, const MidiMessage &, int) override;

	void updateSettings();

	AudioProcessorEditor* createEditor() override;

	void setTrackerID(TrackerType val) { m_tracker_id = val; }
	TrackerType getTrackerID() { return m_tracker_id; }

	void saveCustomParametersToXml(XmlElement* xml) override;
	void loadCustomParametersFromXml() override;
private:
	TrackerType m_tracker_id = TrackerType::kLED;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Trackers);
};

#endif