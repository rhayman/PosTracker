#ifndef TRACKERSEDITOR_H
#define TRACKERSEDITOR_H

#include <memory>
#include <EditorHeaders.h>
#include "Trackers.hpp"
#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
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

	std::vector<std::unique_ptr<Component>> m_UIElements;
	cv::Rect2d getROI() { return m_roi; }
	cv::Ptr<cv::Tracker> getTracker() { return m_tracker; }
private:
	void setROI(cv::Rect2d roi) { m_roi = roi; }
	void setTracker(cv::Ptr<cv::Tracker> t) { m_tracker = t; }

	std::unique_ptr<ComboBox> trackerCombo;
	std::unique_ptr<Label> trackerComboLabel;
	std::unique_ptr<UtilityButton> updateTrackingTypeButton;
	std::unique_ptr<Label> startTrackingLabel;
	std::unique_ptr<cvTracker> m_trackerUI;

	Font m_font;
	cv::Rect2d m_roi;
	cv::Ptr<cv::Tracker> m_tracker;
	Trackers * m_proc;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackersEditor);
};

#endif