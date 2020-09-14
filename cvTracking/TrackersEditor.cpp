#include <opencv2/highgui.hpp>
#include "cvTrackers.hpp"
#include "TrackersEditor.hpp"
#include "../PosTracker/PosTracker.h"

class PosTS;

TrackersEditor::TrackersEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors=true)
	: GenericEditor(parentNode, useDefaultParameterEditors)
{
	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
	MemoryInputStream mis(silk, silksize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	Font font = Font(typeface);
	font.setHeight(9);
	m_font = font;

	m_proc = (Trackers*)getProcessor();

	desiredWidth = 450;

	//add some controls
	trackerCombo = std::make_unique<ComboBox>("trackerCombo");
	trackerCombo->setBounds(5, 25, 80, 20);
	trackerCombo->setTooltip("Tracker to use");
	trackerCombo->setEditableText(false);
	int idx = 1;
	for (auto t : kTrackers ) {
		trackerCombo->addItem(String(t), idx);
		++idx;
	}
	trackerCombo->addListener(this);
	addAndMakeVisible(trackerCombo.get());

	updateTrackingTypeButton = std::make_unique<UtilityButton>("Update", Font ("Small Text", 10, Font::plain));
	updateTrackingTypeButton->setBounds(380, 105, 60, 20);
	updateTrackingTypeButton->setTooltip("Updates the tracking method");
	updateTrackingTypeButton->addListener(this);
	addAndMakeVisible(updateTrackingTypeButton.get());
}

void TrackersEditor::buttonEvent(Button * button) {
	if ( button == updateTrackingTypeButton.get() ) {
		cv::Rect roi_rect;
		cv::Mat frame_clone;
		int idx = trackerCombo->getSelectedId() - 1;
		auto trackerKind = kTrackers[idx];
		// TODO: switch / case this
		if ( trackerKind != "LED" && trackerKind != "Background" && trackerKind != "BackgroundKNN") {
			if ( m_trackerUI ) {
				auto tracker = m_trackerUI->makeTracker();
				setTracker(tracker);
				PosTracker * pos_tracker = (PosTracker*)(m_proc->getSourceNode());
				if ( pos_tracker->isCamReady() ) {
					if ( pos_tracker->isStreaming() ) {
						pos_tracker->playPauseLiveStream(false);
						std::string devName = pos_tracker->getDeviceName();
						Formats * currentFmt = pos_tracker->getCurrentFormat();
						cv::Mat frame;
						frame = cv::Mat(currentFmt->height, currentFmt->width, CV_8UC3, (unsigned char*)pos_tracker->get_frame_ptr());
						frame_clone = frame.clone();
						m_tracker_init = false;

						cv::Rect roi = cv::selectROI("Select ROI", frame_clone);
						if ( ! roi.empty() ) {
							setROI(roi);
							tracker->init(frame, roi);
							m_tracker_init = true;
							pos_tracker->playPauseLiveStream(true);
							cv::destroyWindow("Select ROI");
						}
					}
				}
			}
		}
		if ( trackerKind == "Background" || trackerKind == "BackgroundKNN") {
			if ( m_trackerUI ) {
				auto tracker = m_trackerUI->makeTracker();
				setTracker(tracker);
				cv::Rect roi = cv::Rect(1,1,10,10); // dummy roi checked by PosTracker
				setROI(roi);
				m_tracker_init = false;
				auto bg_tracker = m_trackerUI->makeBackgroundSubtractor();
				setBackgroundSubtractor(bg_tracker);
			}
		}
	}
}

void TrackersEditor::comboBoxChanged(ComboBox * box) {
	if ( box == trackerCombo.get() ) {
		int idx = trackerCombo->getSelectedId() - 1;
		auto trackerKind = kTrackers[idx];
		// clear the containers holding the UI elements
		makeTracker(trackerKind);
		//TODO: MAKE THE OTHER TRACKER TYPES HERE
		updateSettings();
	}
}

void TrackersEditor::makeTracker(const std::string & trackerKind) {
	if ( trackerKind == "Boosting" ) {
		m_proc->setTrackerID(TrackerType::kBoosting);
		m_trackerUI = std::make_unique<BoostingTracker>(this, "Boosting");
		m_trackerUI->makeTrackerUI();
	}
	if ( trackerKind == "KCF" ) {
		m_proc->setTrackerID(TrackerType::kKCF);
		m_trackerUI = std::make_unique<KCFTracker>(this, "KCF");
		m_trackerUI->makeTrackerUI();
	}
	if ( trackerKind == "MedianFlow") {
		m_proc->setTrackerID(TrackerType::kMedianFlow);
		m_trackerUI = std::make_unique<MedianFlow>(this, "MedianFlow");
		m_trackerUI->makeTrackerUI();
	}
	if ( trackerKind == "MIL") {
		m_proc->setTrackerID(TrackerType::kMedianFlow);
		m_trackerUI = std::make_unique<MIL>(this, "MIL");
		m_trackerUI->makeTrackerUI();
	}
	if ( trackerKind == "Background" ) {
		m_proc->setTrackerID(TrackerType::kBACKGROUND);
		m_trackerUI = std::make_unique<Background>(this, "Background");
		m_trackerUI->makeTrackerUI();
		auto bg_tracker = m_trackerUI->makeBackgroundSubtractor();
		setBackgroundSubtractor(bg_tracker);
	}
	if ( trackerKind == "BackgroundKNN" ) {
		m_proc->setTrackerID(TrackerType::kBACKGROUNDKNN);
		m_trackerUI = std::make_unique<BackgroundKNN>(this, "BackgroundKNN");
		m_trackerUI->makeTrackerUI();
		auto bg_tracker = m_trackerUI->makeBackgroundSubtractor();
		setBackgroundSubtractor(bg_tracker);
	}
}

void TrackersEditor::updateSettings() {
	if ( m_trackerUI ) {
		if ( ! m_trackerUI->m_UILabels.empty() ) {
			for ( auto & element : m_trackerUI->m_UILabels ) {
				addAndMakeVisible(element.get());
			}
		}
		if ( ! m_trackerUI->m_UITextEditors.empty() ) {
			for ( auto & element : m_trackerUI->m_UITextEditors ) {
				addAndMakeVisible(element.get());
			}
		}
		if ( ! m_trackerUI->m_UICheckBoxes.empty() ) {
			for ( auto & element : m_trackerUI->m_UICheckBoxes ) {
				addAndMakeVisible(element.get());
			}
		}
		if ( ! m_trackerUI->m_UIComboBoxes.empty() ) {
			for ( auto & element : m_trackerUI->m_UIComboBoxes ) {
				addAndMakeVisible(element.get());
			}
		}
	}
}

void TrackersEditor::loadXmlParams(XmlElement * paramXml) {
	// get the name of the tracker from the xml file
	auto tracker_name = paramXml->getStringAttribute("TrackerName");
	makeTracker(tracker_name.toStdString());
	if ( m_trackerUI  ) {
		forEachXmlChildElement(*paramXml, childElement) {
			forEachXmlChildElement(*childElement, UIElement) {
				auto name = UIElement->getTagName();
				auto val = UIElement->getStringAttribute("Value");
				m_trackerUI->setValue(name, val);
			}
		}
		for (int i = 0; i < kTrackers.size(); ++i) {
			if ( tracker_name == kTrackers[i] )
				trackerCombo->setSelectedId(i+1, dontSendNotification);
		}
	}
	updateSettings();
}
