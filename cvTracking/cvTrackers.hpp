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
	virtual cv::Ptr<cv::BackgroundSubtractor> makeBackgroundSubtractor() { return cv::Ptr<cv::BackgroundSubtractor>(); }
	void addLabel(const std::string & name, const std::vector<int> & bounds);
	void addTextEditor(const std::string & name, const std::vector<int> & bounds, const std::string & initText, const std::string & tooltip);
	void addCheckbox(const std::string & name, const std::vector<int> & bounds, const std::string & tooltip);
	void addComboBox(const std::string & name, const std::vector<int> & bounds, const std::vector<std::string> & entries, const std::string & tooltip);
	Font makeFont();
	// append the params of the tracker to the xml file saved when OE exits
	void saveToXml(XmlElement*);
	void setValue(const String &, const String &);
	// container to hold all the Labels
	std::vector<std::shared_ptr<Component>> m_UILabels;
	// container for all the TextEditors - contain numeric values, ints, floats, doubles
	std::vector<std::shared_ptr<TextEditor>> m_UITextEditors;
	// container for the check-boxes (ToggleButton) - true / false state
	std::vector<std::shared_ptr<ToggleButton>> m_UICheckBoxes;
	// container for the ComboBoxes (lists of items)
	std::vector<std::shared_ptr<ComboBox>> m_UIComboBoxes;
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