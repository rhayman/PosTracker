#ifndef STEREOPOS_H_
#define STEREOPOS_H_

#include <memory>
#include <ProcessorHeaders.h>

class CalibrateCamera;

class StereoPos : public GenericProcessor
{
public:
	StereoPos();
	~StereoPos();

	bool isSource() const override { return false; }
	bool isSink() const override { return false; }
	bool isFilter() const override { return true; }
	bool hasEditor() const override { return true; }

	void process(AudioSampleBuffer& buffer) override;

	bool isReady() override { return true; }

	void testFcn();
	void showCapturedImages(bool);

	void updateSettings();

	AudioProcessorEditor* createEditor() override;

	void saveCustomParametersToXml(XmlElement* xml) override;
	void loadCustomParametersFromXml() override;

private:
	std::unique_ptr<CalibrateCamera> calibrator;

	CriticalSection lock;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoPos);
};
#endif