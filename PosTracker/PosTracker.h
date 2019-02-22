#ifndef POSTRACKER_H_
#define POSTRACKER_H_

#include <iostream>
#include <fstream>
#include <memory>

#include <queue>
#include "common.h"
#include <ProcessorHeaders.h>

class Camera;
class PosTS;
class DisplayMask;

class PosTracker : public GenericProcessor, public Thread
{
public:
	PosTracker();
	~PosTracker();

	bool isSource() const override { return true; }
	bool isSink() const override { return false; }
	bool hasEditor() const override { return true; }

	void process(AudioSampleBuffer& buffer) override;

	void createEventChannels() override;

	AudioProcessorEditor* createEditor() override;
	void updateSettings();
	void setParameter(int paramIdx, float newVal) override;

	void setEnabledState(bool newstate) override { isEnabled = newstate; }
	int getDefaultNumOutputs() const { return 1; }

	bool isReady() override { return true; }

	void run();

	bool isCamReady() { return camReady; }

	void sendTimeStampedPosToMidiBuffer(std::shared_ptr<PosTS> p);

	void createNewCamera(std::string dev_name);
	std::string getDevName() { return m_dev_name; }
	std::string getDeviceName();
	std::vector<std::string> getDeviceList();
	std::string getFormatName();
	void openCamera();
	void stopCamera();
	void startStreaming();
	void showLiveStream(bool val);
	bool isStreaming() { return liveStream; }
	void stopStreaming();
	void startRecording();
	void stopRecording();

	// returns 0 if control is ok, 1 if not
	int getControlValues(__u32, __s32 &, __s32 &, __s32 &);
	void changeExposureTo(int);

	void adjustBrightness(int val);
	void adjustContrast(int val);
	void adjustExposure(int val);
	int getBrightness() { return brightness; }
	int getContrast() { return contrast; }
	int getExposure()  { return exposure; }
	void overlayPath(bool state);
	bool overlayPath() { return path_overlay; }
	void autoExposure(bool state) { auto_exposure = state; }
	bool autoExposure() { return auto_exposure; }

	void adjustVideoMask(BORDER, int val);
	void makeVideoMask();
	int getVideoMask(BORDER);

	std::vector<std::string> getDeviceFormats();
	void setDeviceFormat(std::string);

	int getCurrentCameraIdx() { return currentCameraIdx; }
	void setCurrentCameraIdx(int idx) { currentCameraIdx = idx; }

	std::pair<int,int> getResolution();
	int getCurrentFormatIdx() { return currentFormatIdx; }
	Formats * getCurrentFormat();

	void saveCustomParametersToXml(XmlElement* xml) override;
	void loadCustomParametersFromXml() override;

private:
	std::string m_dev_name;
	std::ofstream ofs;
	bool camReady = false;
	bool threadRunning = false;
	bool liveStream = false;
	int currentCameraIdx = -1;
	int currentFormatIdx = -1;
	std::unique_ptr<Camera> currentCam;

	std::shared_ptr<PosTS> pos_tracker;
	std::unique_ptr<DisplayMask> displayMask;

	CriticalSection lock;
	CriticalSection displayMutex;

	int brightness = 50;
	int contrast = 50;
	int exposure = 10;
	int left_border = 0;
	int right_border = 0;
	int top_border = 0;
	int bottom_border = 0;

	bool auto_exposure = false;
	bool path_overlay = false;

	std::queue<std::shared_ptr<PosTS>> posBuffer;

	const EventChannel* messageChannel{ nullptr };

	Time timer;

	juce::uint32 * xy;
	juce::uint32 * old_xy = nullptr;
	juce::uint32 * new_xy = nullptr;
	struct timeval tv;
	juce::uint32 xy_ts[3];
	struct timespec ts;
	struct timeval tv1;
	struct timeval result;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PosTracker);
};

#endif