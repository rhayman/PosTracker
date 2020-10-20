#ifndef TRACKERSEDITOR_H
#define TRACKERSEDITOR_H

#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/video/background_segm.hpp>
#include <memory>
#include <EditorHeaders.h>
#include "Trackers.hpp"
#include "cvTrackers.hpp"
#include "../common.h"


class TrackersEditor : public GenericEditor, public ComboBox::Listener
{
public:
	TrackersEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors);
	~TrackersEditor() {};

	void editorShown(Label*, TextEditor& textEditorShown) {};

	void comboBoxChanged(ComboBox* cb);
	void buttonEvent(Button* button);
	void updateSettings();
	void update() {};

	cv::Rect2d getROI() { return m_roi; }
	void makeTracker(const std::string &);
	cv::Ptr<cv::Tracker> getTracker() { return m_tracker; }
	cv::Ptr<cv::BackgroundSubtractor> getBackgroundSubtractor() { return m_background_subtractor; }
	bool is_tracker_init() { return m_tracker_init; }
	std::string getTrackerName() { return m_trackerUI->getName(); }
	void saveToXml(XmlElement * elem) { m_trackerUI->saveToXml(elem); }
	std::vector<std::shared_ptr<TextEditor>> getTextComponents() { return m_trackerUI->m_UITextEditors; }
	void loadXmlParams(XmlElement *);
private:
	void setROI(cv::Rect2d roi) { m_roi = roi; }
	void setTracker(cv::Ptr<cv::Tracker> t) { m_tracker = t; }
	void setBackgroundSubtractor(cv::Ptr<cv::BackgroundSubtractor> bg_sub) { m_background_subtractor = bg_sub; }

	std::unique_ptr<cvTracker> m_trackerUI;

	std::unique_ptr<ComboBox> trackerCombo;
	std::unique_ptr<Label> trackerComboLabel;
	std::unique_ptr<UtilityButton> updateTrackingTypeButton;
	std::unique_ptr<Label> startTrackingLabel;
	cv::Ptr<cv::BackgroundSubtractor> m_background_subtractor;

	Font m_font;
	cv::Rect2d m_roi;
	cv::Ptr<cv::Tracker> m_tracker;
	Trackers * m_proc;
	bool m_tracker_init = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackersEditor);
};

#endif