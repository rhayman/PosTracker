#ifndef STEREOPOS_H_
#define STEREOPOS_H_

#include <memory>
#include <vector>
#include <ProcessorHeaders.h>

class CalibrateCamera;
class PosTracker;
class Formats;

class StereoPos : public GenericProcessor, public Thread
{
public:
	StereoPos();
	~StereoPos();

	bool isSource() const override { return false; }
	bool isSink() const override { return false; }
	bool isFilter() const override { return true; }
	bool hasEditor() const override { return true; }

	bool isReady() override { return true; }
	void run();
	void startStreaming();
	void stopStreaming();
	void process(AudioSampleBuffer& buffer) override { checkForEvents(); }

	void doStereoCalibration();
	bool calibrate(CalibrateCamera * camera_1, CalibrateCamera * camera_2);
	void handleEvent(const EventChannel *, const MidiMessage &, int) override;

	void showCapturedImages(bool);

	void updateSettings();

	AudioProcessorEditor* createEditor() override;

	void saveCustomParametersToXml(XmlElement* xml) override;
	void loadCustomParametersFromXml() override;

private:
	std::vector<std::unique_ptr<CalibrateCamera>> calibrators;
	std::vector<PosTracker*> m_trackers;

	unsigned int nImagesToCapture = 0;
	unsigned int nSecondsBetweenCaptures = 1;

	bool m_threadRunning = false;
	CriticalSection lock;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoPos);
};
#endif