#ifndef CVTRACKERS_H_
#define CVTRACKERS_H_

#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/video/background_segm.hpp>
#include <EditorHeaders.h>

class cvTracker {
public:
	cvTracker(GenericEditor * parent, std::string name) : m_parent(parent), m_name(name) {};
	virtual ~cvTracker() {};
	std::string getName() { return m_name; }
	virtual void makeTrackerUI() = 0;
	virtual cv::Ptr<cv::Tracker> makeTracker() = 0;
	virtual cv::Ptr<cv::BackgroundSubtractor> makeBackgroundSubtractor() {};
	void addLabelToUIElements(const std::string & name, const std::vector<int> & bounds);
	void addTextEditorToUIElements(const std::string & name, const std::vector<int> & bounds, const std::string & initText, const std::string & tooltip);
	void addCheckboxTouiElements(const std::string & name, const std::vector<int> & bounds, const std::string & tooltip);
	void addComboBoxToUIElements(const std::string & name, const std::vector<int> & bounds, const std::vector<std::string> & entries, const std::string & tooltip);
	Font makeFont();
	// container to hold all the UI elements
	std::vector<std::shared_ptr<Component>> m_UIElements;
protected:
	GenericEditor * m_parent;
	std::string m_name;
private:
};

class BoostingTracker : public cvTracker {
public:
	BoostingTracker(GenericEditor * parent, std::string name) : cvTracker(parent, name) {};
	~BoostingTracker() {};
	virtual void makeTrackerUI() override;
	virtual cv::Ptr<cv::Tracker> makeTracker() override;
};

class KCFTracker : public cvTracker {
public:
	KCFTracker(GenericEditor * parent, std::string name) : cvTracker(parent, name) {};
	~KCFTracker() {};
	virtual void makeTrackerUI() override;
	virtual cv::Ptr<cv::Tracker> makeTracker() override;
};

class MedianFlow : public cvTracker {
public:
	MedianFlow(GenericEditor * parent, std::string name) : cvTracker(parent, name) {};
	~MedianFlow() {};
	virtual void makeTrackerUI() override;
	virtual cv::Ptr<cv::Tracker> makeTracker() override;
};

class MIL : public cvTracker {
public:
	MIL(GenericEditor * parent, std::string name) : cvTracker(parent, name) {};
	~MIL() {};
	virtual void makeTrackerUI() override;
	virtual cv::Ptr<cv::Tracker> makeTracker() override;
};

class Background : public cvTracker {
public:
	Background(GenericEditor * parent, std::string name) : cvTracker(parent, name) {};
	~Background() {};
	virtual void makeTrackerUI() override;
	virtual cv::Ptr<cv::Tracker> makeTracker() override;
	virtual cv::Ptr<cv::BackgroundSubtractor> makeBackgroundSubtractor() override;
};

class BackgroundKNN : public cvTracker {
public:
	BackgroundKNN(GenericEditor * parent, std::string name) : cvTracker(parent, name) {};
	~BackgroundKNN() {};
	virtual void makeTrackerUI() override;
	virtual cv::Ptr<cv::Tracker> makeTracker() override;
	virtual cv::Ptr<cv::BackgroundSubtractor> makeBackgroundSubtractor() override;
};

#endif