#ifndef POSTRACKER_H_
#define POSTRACKER_H_

#include <iostream>
#include <fstream>
#include <memory>

#include <queue>
#include "../common.h"
#include <ProcessorHeaders.h>

class CameraBase;
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
	void playPauseLiveStream(bool val) { m_switchPausePlay = val; }

	// returns 0 if control is ok, 1 if not
	int getControlValues(__u32, __s32 &, __s32 &, __s32 &);
	void changeExposureTo(int);

	void adjustBrightness(int val);
	void adjustContrast(int val);
	void adjustExposure(int val);
	void adjustThreshold(int val);
	int getBrightness() { return brightness; }
	int getContrast() { return contrast; }
	int getExposure()  { return exposure; }
	int getThreshold() { return threshold; }
	void overlayPath(bool state);
	bool overlayPath() { return path_overlay; }
	void autoExposure(bool state) { auto_exposure = state; }
	bool autoExposure() { return auto_exposure; }
	
	// Two spot tracking stuff
	void doTwoSpotTracking(bool val) { m_doTwoSpotTracking = val; }
	bool doTwoSpotTracking() { return m_doTwoSpotTracking; }
	void twoSpotMinDistance(unsigned int val) { m_twoSpotMinDist = val; }
	unsigned int twoSpotMinDistance() { return m_twoSpotMinDist; }
	void twoSpotSmallSpotSize(unsigned int val) { m_twoSpotSmallSpotSize = val; }
	unsigned int twoSpotSmallSpotSize() { return m_twoSpotSmallSpotSize; }
	void twoSpotBigSpotSize(unsigned int val) { m_twoSpotBigSpotSize = val; }
	unsigned int twoSpotBigSpotSize() { return m_twoSpotBigSpotSize; }

	void adjustVideoMask(BORDER, int val);
	void makeVideoMask();
	int getVideoMask(BORDER);

	std::vector<std::string> getDeviceFormats();
	void setDeviceFormat(std::string);

	std::shared_ptr<CameraBase> getCurrentCamera() { return currentCam; }
	int getCurrentCameraIdx() { return currentCameraIdx; }
	void setCurrentCameraIdx(int idx) { currentCameraIdx = idx; }

	std::pair<int,int> getResolution();
	unsigned int getFrameRate();
	int getCurrentFormatIdx() { return currentFormatIdx; }
	Formats * getCurrentFormat();

	void * get_frame_ptr() { return m_frame_ptr; }

	void saveCustomParametersToXml(XmlElement* xml) override;
	void loadCustomParametersFromXml() override;

	std::shared_ptr<PosTS> pos_tracker;
private:
	std::string m_dev_name;
	std::ofstream ofs;
	bool camReady = false;
	bool liveStream = false;
	int currentCameraIdx = -1;
	int currentFormatIdx = -1;
	std::shared_ptr<CameraBase> currentCam;

	std::unique_ptr<DisplayMask> displayMask;

	void * m_frame_ptr;

	CriticalSection lock;
	CriticalSection displayMutex;

	int brightness = 50;
	int contrast = 50;
	int exposure = 10;
	int threshold = 100;
	int left_border = 0;
	int right_border = 0;
	int top_border = 0;
	int bottom_border = 0;

	bool auto_exposure = false;
	bool path_overlay = false;
	bool m_switchPausePlay = true;
	bool m_doTwoSpotTracking = false;
	unsigned int m_twoSpotMinDist = 10;
	unsigned int m_twoSpotBigSpotSize = 300;
	unsigned int m_twoSpotSmallSpotSize = 100;

	std::queue<std::shared_ptr<juce::uint32*>> posBuffer;

	const EventChannel* messageChannel{ nullptr };

	Time timer;

	juce::uint32 * xy;
	juce::uint32 * old_xy = nullptr;
	juce::uint32 * new_xy = nullptr;
	juce::uint32 xy_ts[4];
	timespec ts;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PosTracker);
};

#endif