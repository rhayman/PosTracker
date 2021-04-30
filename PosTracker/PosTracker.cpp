#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/imgproc.hpp>
#include "PosTracker.h"
#include "PosTrackerEditor.h"
#include "CameraCV.h"
#include "Camera.h"
#include "../cvTracking/TrackersEditor.hpp"

#include <array>
#include <vector>

/*
A class for decorating the video output frame with a windowed border,
the path of the animal, and maybe other stuff if I can be bothered...
*/
class DisplayMask
{
public:
	DisplayMask() {};
	DisplayMask(cv::Mat mask) : m_displayMask(m_mask) {};
	~DisplayMask() {};
	void makeMask(int width, int height)
	{
		m_mask = cv::Mat::ones(width, height, CV_8UC1);
		cv::rectangle(m_mask, cv::Point(m_left_mat_edge, m_top_mat_edge), cv::Point(m_right_mat_edge, m_bottom_mat_edge), cv::Scalar(0), -1, 8, 0);
		m_mask.copyTo(m_displayMask);
		m_mask.copyTo(m_pathFrame);
		cv::cvtColor(m_displayMask, m_displayMask, cv::COLOR_GRAY2BGR);
		cv::cvtColor(m_pathFrame, m_pathFrame, cv::COLOR_GRAY2BGR);

	}
	cv::Mat getSingleChannelMask() { return m_mask; }
	cv::Mat getMask() { return m_displayMask; }
	cv::Mat getPathFrame() { return m_pathFrame; }
	void setPathFrame(cv::Scalar p) { m_pathFrame = p; }
	void setEdge(BORDER edge, int val)
	{
		switch (edge) {
			case BORDER::LEFT: m_left_mat_edge = val; break;
			case BORDER::RIGHT: m_right_mat_edge = val; break;
			case BORDER::TOP: m_top_mat_edge = val; break; // co-ords switched
			case BORDER::BOTTOM: m_bottom_mat_edge = val; break;
		}
	};
	int getEdge(BORDER edge)
	{
		switch (edge) {
			case BORDER::LEFT: return m_left_mat_edge;
			case BORDER::RIGHT: return m_right_mat_edge;
			case BORDER::TOP: return m_top_mat_edge;// co-ords switched
			case BORDER::BOTTOM: return m_bottom_mat_edge;
			default: return m_top_mat_edge;
		}
	};
	cv::Rect getROIRect()
	{
		return cv::Rect(m_left_mat_edge, m_top_mat_edge, m_right_mat_edge-m_left_mat_edge, m_bottom_mat_edge-m_top_mat_edge);
	};
private:
	cv::Mat m_displayMask;
	cv::Mat m_pathFrame;
	cv::Mat m_mask;
	// values for excluding points outside the mask for use in PosTS class below
	// and the display of the bounding box for the windowed video output
	int m_left_mat_edge = 0;
	int m_right_mat_edge = 0;
	int m_top_mat_edge = 0;
	int m_bottom_mat_edge = 0;
};


class PosTS
{
public:
	PosTS() {};
	PosTS(struct timeval tv, cv::Mat & src) : m_tv(tv), m_src(src) {};
	~PosTS() {};
	void setROIRect(cv::Rect roi) { roi_rect = roi; }
	void setMask(cv::Mat m) { mask = m; }
	void setTV(struct timeval t) { m_tv = t; }
	void setThreshold(int val) { m_thresh = val; }
	void setTracker(cv::Ptr<cv::Tracker> t) { m_tracker = t; }
	void setBackgroundSubtractor(cv::Ptr<cv::BackgroundSubtractor> bg) { m_background_sub = bg; }
	void doDetection(const TrackerType method, const cv::Mat & frame, cv::Rect2d & bounding_box) {
		if ( ! frame.empty() ) {
			if ( method == TrackerType::kLED )
				singleLEDDetection(frame);
			else if ( method == TrackerType::kTwoSpotTracking )
				doubleLEDDetection(frame);
			else if ( method == TrackerType::kBACKGROUND || method == TrackerType::kBACKGROUNDKNN ) {
				// specialised method not using opencv's Tracker API
				if ( m_background_sub ) {
					cv::Mat fg_mask;
					// get the frame roi
					cv::Mat roi = frame(roi_rect);
					m_background_sub->apply(roi, fg_mask);
					cv::Mat labels, stats, centroids;
					int nlabels = cv::connectedComponentsWithStats(fg_mask, labels, stats, centroids, 8, CV_32S, cv::CCL_WU);
					cv::Size stats_size = stats.size();
					if ( stats_size.height > 1 ) {
						int maxpix = 0;
						int biggestComponent = 0;
						for (int i = 1; i < stats_size.height; ++i)
						{
							if ( stats.at<int>(i, cv::CC_STAT_AREA) > maxpix ) {
								maxpix = stats.at<int>(i, cv::CC_STAT_AREA);
								biggestComponent = i;
							}
						}
						double x_centroid, y_centroid;
						x_centroid = centroids.at<double>(biggestComponent, 0);
						y_centroid = centroids.at<double>(biggestComponent, 1);
						maxloc.x = static_cast<int>(x_centroid) + roi_rect.x;
						maxloc.y = static_cast<int>(y_centroid) + roi_rect.y;
						m_xy[0] = (juce::uint32)maxloc.x;
						m_xy[1] = (juce::uint32)maxloc.y;
					}
					else
						fallbackDetection(fg_mask);
				}
			}
			else {
				if ( trackerIsInit ) {
					m_tracker->update(frame, bounding_box);
					// fill out the x,y data for saving open-ephys data stream
					auto centre_x = bounding_box.x + (bounding_box.width/2.0);
					auto centre_y = bounding_box.y + (bounding_box.height/2.0);
					maxloc.x = centre_x;
					maxloc.y = centre_y;
					m_xy[0] = (juce::uint32)maxloc.x;
					m_xy[1] = (juce::uint32)maxloc.y;
				}
				if ( bounding_box.empty() ) {
					//DO FALL BACK METHOD
					cv::extractChannel(frame, red_channel, 0);
					cv::Mat roi = red_channel(roi_rect);
					cv::threshold(roi, roi, m_thresh, 1000, cv::THRESH_BINARY);
					fallbackDetection(roi);
				}
			}
		}
	};
	// Deal with a directional LED setup ie one small and one large LED
	void doubleLEDDetection(const cv::Mat & frame) {
		// if ( ! frame.empty() ) {
			cv::extractChannel(frame, red_channel, 0);
			cv::Mat roi = red_channel(roi_rect);
			cv::threshold(roi, roi, m_thresh, 1000, cv::THRESH_BINARY);
			cv::Mat labels, stats, centroids;
			int nlabels = cv::connectedComponentsWithStats(roi, labels, stats, centroids, 8, CV_32S, cv::CCL_WU);
			cv::Size stats_size = stats.size();
			if ( stats_size.height > 2 ) {
				int maxpix = 0;
				int biggestComponent = 0;
				for (int i = 1; i < stats_size.height; ++i) // starts at 1 as 0 is background
				{
					auto area = stats.at<int>(i, cv::CC_STAT_AREA);
					if ( area > maxpix && area > bigSpotSize ) {
						maxpix = stats.at<int>(i, cv::CC_STAT_AREA);
						biggestComponent = i;
						double x_centroid, y_centroid;
			
						x_centroid = centroids.at<double>(biggestComponent, 0);
						y_centroid = centroids.at<double>(biggestComponent, 1);
						maxloc.x = static_cast<int>(x_centroid) + roi_rect.x;
						maxloc.y = static_cast<int>(y_centroid) + roi_rect.y;
					}
				}
				maxpix = 0;
				for (int i = 1; i < stats_size.height; ++i) // starts at 1 as 0 is background
				{
					if ( i != biggestComponent ) {
						auto area = stats.at<int>(i, cv::CC_STAT_AREA);
						if ( area > maxpix && area < bigSpotSize && area > smallSpotSize ) {
							maxpix = stats.at<int>(i, cv::CC_STAT_AREA);
							biggestComponent = i;
							double x_centroid, y_centroid;
				
							x_centroid = centroids.at<double>(biggestComponent, 0);
							y_centroid = centroids.at<double>(biggestComponent, 1);
							maxloc2.x = static_cast<int>(x_centroid) + roi_rect.x;
							maxloc2.y = static_cast<int>(y_centroid) + roi_rect.y;
						}
					}
				}

			}
			m_xy[0] = (juce::uint32)maxloc.x;
			m_xy[1] = (juce::uint32)maxloc.y;
			m_xy[2] = (juce::uint32)maxloc2.x;
			m_xy[3] = (juce::uint32)maxloc2.y;
		// }
	};
	void singleLEDDetection(const cv::Mat & frame)
	{
		if ( ! frame.empty() ) {
			cv::extractChannel(frame, red_channel, 0);
			cv::Mat roi = red_channel(roi_rect);
			cv::threshold(roi, roi, m_thresh, 1000, cv::THRESH_BINARY);
			cv::Mat labels, stats, centroids;
			int nlabels = cv::connectedComponentsWithStats(roi, labels, stats, centroids, 8, CV_32S, cv::CCL_WU);
			cv::Size stats_size = stats.size();
			if ( stats_size.height > 1 ) {
				int maxpix = 0;
				int biggestComponent = 0;
				for (int i = 1; i < stats_size.height; ++i) // starts at 1 as 0 is background
				{
					if ( stats.at<int>(i, cv::CC_STAT_AREA) > maxpix ) {
						maxpix = stats.at<int>(i, cv::CC_STAT_AREA);
						biggestComponent = i;
					}
				}
				double x_centroid, y_centroid;
				x_centroid = centroids.at<double>(biggestComponent, 0);
				y_centroid = centroids.at<double>(biggestComponent, 1);
				maxloc.x = static_cast<int>(x_centroid) + roi_rect.x;
				maxloc.y = static_cast<int>(y_centroid) + roi_rect.y;
			}
			else
				fallbackDetection(roi);

			m_xy[0] = (juce::uint32)maxloc.x;
			m_xy[1] = (juce::uint32)maxloc.y;
			m_xy[2] = (juce::uint32)maxloc2.x;
			m_xy[3] = (juce::uint32)maxloc2.y;
		}
	};
	void fallbackDetection(cv::Mat roi) {
		cv::minMaxLoc(roi, NULL, NULL, NULL, &maxloc, ~roi);
		maxloc.x = maxloc.x + roi_rect.x;
		maxloc.y = maxloc.y + roi_rect.y;
		m_xy[0] = (juce::uint32)maxloc.x;
		m_xy[1] = (juce::uint32)maxloc.y;
	};
	cv::Mat processFrame(cv::Mat & frame)
	{
		cv::Mat cpy;
		frame.copyTo(cpy);
		cv::cvtColor(cpy, cpy, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(cpy, cpy, cv::Size(3,3), 3.0, 2.0);
		cv::threshold(cpy, cpy, 180, 1000, cv::THRESH_BINARY);
		cv::Mat kern = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
		cv::erode(cpy, cpy, kern, cv::Point(-1,-1), 1);
		return cpy;
	};

	void blobDetect(std::vector<cv::Point2f> & centre, std::vector<float> & radius)
	{
		cv::Mat m_blah = processFrame(m_src);
		std::vector<std::vector<cv::Point>> cnts;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(m_blah, cnts, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		cv::Point2f __centre;
		float __radius;
		float old_radius = 0;
		int j = 0;
		for (int i = 0; i < cnts.size(); ++i)
		{
			cv::minEnclosingCircle(cnts[i], __centre, __radius);
			centre.push_back(__centre);
			radius.push_back(__radius);
			m_xy[0] = (juce::uint32)__centre.x;
			m_xy[1] = (juce::uint32)__centre.y;
		}
	};

	struct timeval getTimeVal() { return m_tv; }
	juce::uint32 * getPos()
	{
		return m_xy;
	};
	friend std::ostream & operator<<(std::ostream & out, const PosTS & p)
	{
		out << "\t" << p.m_xy[0] << "\t" << p.m_xy[1] << std::endl;
		return out;
	};
	cv::Mat kern = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
	cv::Mat red_channel, m_src;
	cv::Point maxloc, maxloc2;
	juce::uint32 m_xy[4] = {}; // zero-initialize the array
	struct timeval m_tv;
	int m_thresh = 100;
	std::vector<cv::KeyPoint> kps;
	cv::Rect roi_rect;
	cv::Mat mask;
	cv::Ptr<cv::Tracker> m_tracker;
	cv::Ptr<cv::BackgroundSubtractor> m_background_sub;
	bool trackerIsInit = false;
	bool useTwoSpotTracking = false;
	unsigned int twoSpotMinDistance = 10;
	unsigned int bigSpotSize = 300;
	unsigned int smallSpotSize = 100;
private:
};

PosTracker::PosTracker() : GenericProcessor("Pos Tracker"), Thread("PosTrackerThread")
{
	setProcessorType (PROCESSOR_TYPE_SOURCE);
	sendSampleCount = false; // only sending events
	displayMask = std::make_unique<DisplayMask>();
}

PosTracker::~PosTracker()
{
	std::cout << "PosTracker dtor called\n";
	if ( camReady ) {
		std::cout << "Stopping camera...\n";
		stopCamera();
	}
	if ( isThreadRunning () ) {
		std::cout << "Stopping thread...\n";
		signalThreadShouldExit();
		stopThread(1000);
	}
	displayMask.reset();
}

void PosTracker::sendTimeStampedPosToMidiBuffer(std::shared_ptr<PosTS> p)
{
	xy = p->getPos();
	xy_ts[0] = xy[0];
	xy_ts[1] = xy[1];
	xy_ts[2] = xy[2];
	xy_ts[3] = xy[3];
	BinaryEventPtr event = BinaryEvent::createBinaryEvent(messageChannel, CoreServices::getGlobalTimestamp(), xy_ts, sizeof(juce::uint32)*4);
	addEvent(messageChannel, event, 0);
}

void PosTracker::process(AudioSampleBuffer& buffer)
{
	setTimestampAndSamples(CoreServices::getGlobalTimestamp(), 0);
	// lock.enter();
	while ( ! posBuffer.empty() )
	{
		std::shared_ptr<PosTS> p = std::move(posBuffer.front());
		xy = p->getPos();
		xy_ts[0] = xy[0];
		xy_ts[1] = xy[1];
		xy_ts[2] = xy[2];
		xy_ts[3] = xy[3];
		BinaryEventPtr event = BinaryEvent::createBinaryEvent(messageChannel, CoreServices::getGlobalTimestamp(), xy_ts, sizeof(juce::uint32)*4);
		addEvent(messageChannel, event, 0);
		// sendTimeStampedPosToMidiBuffer(std::move(p));
		posBuffer.pop();
	}
	// lock.exit();
}

void PosTracker::createEventChannels()
{
	auto camera_framerate = getFrameRate();
	EventChannel * chan = new EventChannel(EventChannel::UINT32_ARRAY, 1, 4, CoreServices::getGlobalSampleRate(), this);
	chan->setName(m_dev_name);
	chan->setDescription("x-y position of animal");
	chan->setIdentifier("external.position.rawData");
	eventChannelArray.add(chan);
	messageChannel = chan;
}

void PosTracker::startRecording()
{
	openCamera();
	startStreaming();
}

int PosTracker::getControlValues(__u32 id, __s32 & min, __s32 & max, __s32 & step)
{
	if ( camReady )
		return currentCam->get_control_values(id, min, max, step);
	else
		return 1;
}

void PosTracker::changeExposureTo(int autoOrManual)
{
	if ( camReady )
		currentCam->switch_exposure_type(autoOrManual);
}
void PosTracker::stopRecording()
{
	stopStreaming();
}

void PosTracker::openCamera()
{
	if ( ! currentCam->ready() ) {
		if ( currentCam->open_device() == 0)
			camReady = true;
		else
			camReady = false;
	}
	if ( ! currentCam->initialized() ) {
		if ( currentCam->init_device() == 0)
			camReady = true;
		else
			camReady = false;
	}
	if ( ! currentCam->started() ) {
		if ( currentCam->start_device() == 0)
			camReady = true;
		else
			camReady = false;
	}
}

void PosTracker::stopCamera()
{
	if ( currentCam->started() ) {
		std::cout << "Stopping device...\n";
		currentCam->stop_device();
	}
	if ( currentCam->initialized() ) {
		std::cout << "Uninit device...\n";
		currentCam->uninit_device();
	}
	if ( currentCam->ready() ) {
		std::cout << "Closing device...\n";
		currentCam->close_device();
	}
}

void PosTracker::startStreaming()
{
	if ( camReady )
	{
		if ( liveStream == true )
		{
			cv::namedWindow(currentCam->get_dev_name(), cv::WINDOW_NORMAL & cv::WND_PROP_ASPECT_RATIO & cv::WINDOW_GUI_NORMAL);
		}
		posBuffer = std::queue<std::shared_ptr<PosTS>>{}; // clear the buffer
		startThread(); // calls run()
	}
}

void PosTracker::stopStreaming()
{
	if ( isThreadRunning() )
		stopThread(1000);
	if ( camReady )
		stopCamera();
	liveStream = false;
	posBuffer = std::queue<std::shared_ptr<PosTS>>{};
}

void PosTracker::showLiveStream(bool val)
{
	liveStream = val;
	if ( liveStream == false ){
		cv::destroyWindow(currentCam->get_dev_name());
	}
}

void PosTracker::run()
{
	cv::Mat frame,  roi;
	timeval tv;
	std::vector<cv::Point2d> pts{2};
	unsigned int count = 0;
	if ( ! displayMask->getPathFrame().empty() ) {
		displayMask->setPathFrame(cv::Scalar(0));
	}

	auto ed = static_cast<PosTrackerEditor*>(getEditor());
		
	pos_tracker = std::make_shared<PosTS>(tv, frame);

	// check if we have a destination node of Tracking API...
	Trackers * tracker_proc = (Trackers*)getDestNode();
	TrackerType kind_of_tracker = TrackerType::kLED;
	if ( doTwoSpotTracking() ) {
		kind_of_tracker = TrackerType::kTwoSpotTracking;
		pos_tracker->useTwoSpotTracking = true;
	}
	else
		pos_tracker->useTwoSpotTracking = false;

	if ( tracker_proc && (tracker_proc->getName() == String("Tracker API") ) ) {
		kind_of_tracker = tracker_proc->getTrackerID();
	}

	bool cv_tracker_init = false;

	while ( isThreadRunning() )
	{
		if ( threadShouldExit() )
			return;
		if ( camReady )
		{
			juce::int64 st = cv::getTickCount();
			currentCam->read_frame(frame, tv);
			pos_tracker->setTV(tv);

			if ( !frame.empty() )
			{
				// lock.enter();
				m_frame_ptr = static_cast<void*>(frame.data);

				// provide the PosTS instance with masks etc
				cv::Mat displayMask_mask = displayMask->getMask();
				pos_tracker->setMask(displayMask_mask);
				pos_tracker->setROIRect(displayMask->getROIRect());

				// TESTING TRACKING WITH CV TRACKER API
				if ( tracker_proc && (tracker_proc->getName() == String("Tracker API") ) ) {
					auto tracker_proc_ed = static_cast<TrackersEditor*>(tracker_proc->getEditor());
					kind_of_tracker = tracker_proc->getTrackerID();
					cv_tracker_init = tracker_proc_ed->is_tracker_init();
					pos_tracker->trackerIsInit = cv_tracker_init;
					if ( tracker_proc && ! cv_tracker_init) {
						auto cv_tracker = tracker_proc_ed->getTracker();
						auto bounding_box = tracker_proc_ed->getROI();

						if ( cv_tracker && ! bounding_box.empty() ) {
							pos_tracker->setTracker(cv_tracker);
						}
						if ( kind_of_tracker == TrackerType::kBACKGROUND ) {
							pos_tracker->setBackgroundSubtractor(tracker_proc_ed->getBackgroundSubtractor());
						}
						else if ( kind_of_tracker == TrackerType::kBACKGROUNDKNN ) {
							pos_tracker->setBackgroundSubtractor(tracker_proc_ed->getBackgroundSubtractor());
						}
					}
					cv_tracker_init = true;
				}
				if ( doTwoSpotTracking() ) {
					pos_tracker->twoSpotMinDistance = twoSpotMinDistance();
					pos_tracker->bigSpotSize = twoSpotBigSpotSize();
					pos_tracker->smallSpotSize = twoSpotSmallSpotSize();
				}
				// Do the actual detection using whatever method the user asked for
				cv::Rect2d bb;
				pos_tracker->doDetection(kind_of_tracker, frame, bb);

				if ( liveStream == true )
				{
					auto xy = pos_tracker->getPos();
					pts[count%2] = cv::Point2d(double(xy[0]), double(xy[1]));
					ed->setInfoValue(InfoLabelType::XPOS, (double)xy[0]);
					ed->setInfoValue(InfoLabelType::YPOS, (double)xy[1]);
					cv::bitwise_and(frame, displayMask_mask, frame, displayMask->getSingleChannelMask());

					if ( pts[0].x > getVideoMask(BORDER::LEFT) && pts[0].y > getVideoMask(BORDER::TOP) &&
						pts[1].x > getVideoMask(BORDER::LEFT) && pts[1].y > getVideoMask(BORDER::TOP) && path_overlay == true )
					{
						cv::Mat pathFrame = displayMask->getPathFrame();
						cv::line(pathFrame, pts[0], pts[1], cv::Scalar(0,255,0), 2, cv::LINE_8);
						cv::addWeighted(frame, 1.0, pathFrame, 0.5, 0.0, frame);
					}
					// Draw a little red square to denote the tracked position...
					cv::rectangle(frame, cv::Point(double(xy[0])-3, double(xy[1])-3), cv::Point(double(xy[0])+3, double(xy[1])+3), cv::Scalar(0,0,255), -1,1);
					// and a little yellow one in the case of two-spot tracking
					if ( doTwoSpotTracking() ) {
						cv::rectangle(frame, cv::Point(double(xy[2])-3, double(xy[3])-3), cv::Point(double(xy[2])+3, double(xy[3])+3), cv::Scalar(0,255,255), -1,1);
					}
					if ( ! bb.empty() ) {
						cv::rectangle(frame, bb, cv::Scalar(255, 0, 0), 1, 1);
					}
					if ( cv::waitKey(1) == 32 ) {
						m_switchPausePlay = !m_switchPausePlay;
					}
					if ( m_switchPausePlay )
						cv::imshow(currentCam->get_dev_name(), frame);

					double fps = cv::getTickFrequency() / (cv::getTickCount() - st);
					ed->setInfoValue(InfoLabelType::FPS, fps);
					++count;
				}
				if ( CoreServices::getRecordingStatus() )
					posBuffer.push(pos_tracker);

				// lock.exit();
			}
		}
	}
	std::cout << std::endl;
}

void PosTracker::adjustBrightness(int val)
{
	if ( currentCam )
		currentCam->set_control_value(V4L2_CID_BRIGHTNESS, val);
	brightness = val;
}

void PosTracker::adjustContrast(int val)
{
	if ( currentCam )
		currentCam->set_control_value(V4L2_CID_CONTRAST, val);
	contrast = val;
}

void PosTracker::adjustExposure(int val)
{
	if ( currentCam )
		currentCam->set_control_value(V4L2_CID_EXPOSURE_ABSOLUTE, val);
	exposure = val;
}

void PosTracker::adjustThreshold(int val) {
	if ( currentCam ) {
		if ( pos_tracker )
			pos_tracker->setThreshold(val);
	}
}

void PosTracker::adjustVideoMask(BORDER edge, int val)
{
	displayMask->setEdge(edge, val);
}

void PosTracker::makeVideoMask()
{
	if ( currentCam )
	{
		if ( currentCam->get_current_format() )
		{
			lock.enter();
			std::pair<int, int> res = getResolution();
			displayMask->makeMask(res.second, res.first);
			lock.exit();
		}
	}
}

int PosTracker::getVideoMask(BORDER edge)
{
	return displayMask->getEdge(edge);
}

void PosTracker::overlayPath(bool state)
{
	 path_overlay = state;
	 if ( ! displayMask->getPathFrame().empty() )
		displayMask->setPathFrame(cv::Scalar(0));
}

AudioProcessorEditor* PosTracker::createEditor()
{
	editor = new PosTrackerEditor(this, true);
	return editor;
}

void PosTracker::updateSettings()
{
	if ( editor != NULL )
		editor->updateSettings();
}

void PosTracker::setParameter(int paramIdx, float newVal)
{}


std::vector<std::string> PosTracker::getDeviceFormats()
{
	if ( ! currentCam->ready() )
		currentCam->open_device();
	auto f = currentCam->get_formats(); // clears the Container holding the descriptions of available camera formats
	std::cout << "f size = " << f.size() << std::endl;
	return currentCam->get_format_descriptions();
}

void PosTracker::setDeviceFormat(std::string format)
{
	if ( ! currentCam->ready() ) {
		if ( currentCam->open_device() == 0)
			camReady = true;
		else
			camReady = false;
	}
	std::vector<Formats*> formats = currentCam->get_formats();
	for ( auto & f : formats)
	{
		if ( f->get_description().compare(format) == 0 )
			currentCam->set_format(format);
	}
	camReady = true;
}

Formats * PosTracker::getCurrentFormat()
{
	return currentCam->get_current_format();
}

std::string PosTracker::getDeviceName()
{
	if ( currentCam ) {
		m_dev_name = currentCam->get_dev_name();
		return m_dev_name;
	}
	else
		return "";
}

std::string PosTracker::getFormatName()
{
	std::string name;
	if ( currentCam )
		name = currentCam->get_format_name();
	return name;
}

std::vector<std::string> PosTracker::getDeviceList()
{
	std::vector<std::string> devices = CameraBase::get_devices();
	return devices;
}

void PosTracker::createNewCamera(std::string dev_name)
{
	std::cout << "dev_name = " << dev_name << std::endl;
	if ( currentCam )
	{
		if ( currentCam->ready() )
		{
			currentCam->stop_device();
			currentCam->uninit_device();
			currentCam->close_device();
		}
		currentCam.reset();
	}
	std::vector<std::string> devices = Camera::get_devices();
	std::cout << "devices size = " << devices.size() << std::endl;
	for ( auto & dev : devices )
	{
		std::cout << "dev = " << dev << std::endl;
		if ( dev.compare(dev_name) == 0 ) {
			#ifdef _WIN32
			currentCam = std::make_shared<CameraCV>(dev_name);
			#endif
			#ifdef __unix__
			currentCam = std::make_shared<Camera>(dev_name);
			#endif
		}
	}
}

std::pair<int,int> PosTracker::getResolution()
{
	if ( currentCam )
	{
		if ( currentCam->get_current_format() )
		{
			auto format = currentCam->get_current_format();
			return std::make_pair<int,int>(static_cast<int>(format->width), static_cast<int>(format->height));
		}
	}
	return std::make_pair<int,int>(1, 1);
}

unsigned int PosTracker::getFrameRate() {
	if ( currentCam ) {
		if ( currentCam->get_current_format() ) {
			auto format = currentCam->get_current_format();
			return format->get_framerate();
		}
	}
	return 0;
}

void PosTracker::saveCustomParametersToXml(XmlElement* xml)
{
	xml->setAttribute("Type", "PosTracker");
	XmlElement * paramXml = xml->createNewChildElement("Parameters");
	paramXml->setAttribute("Brightness", getBrightness());
	paramXml->setAttribute("Contrast", getContrast());
	paramXml->setAttribute("Exposure", getExposure());
	if ( displayMask ) {
		paramXml->setAttribute("LeftBorder", displayMask->getEdge(BORDER::LEFT));
		paramXml->setAttribute("RightBorder", displayMask->getEdge(BORDER::RIGHT));
		paramXml->setAttribute("TopBorder", displayMask->getEdge(BORDER::TOP));
		paramXml->setAttribute("BottomBorder", displayMask->getEdge(BORDER::BOTTOM));
	}
	paramXml->setAttribute("AutoExposure", auto_exposure);
	paramXml->setAttribute("OverlayPath", path_overlay);

	XmlElement * deviceXml = xml->createNewChildElement("Devices");
	deviceXml->setAttribute("Camera", getDeviceName());
	deviceXml->setAttribute("Format", getFormatName());
}

void PosTracker::loadCustomParametersFromXml()
{
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Parameters")
	{
		if ( paramXml->hasAttribute("Brightness") )
			brightness = paramXml->getIntAttribute("Brightness");
		if ( paramXml->hasAttribute("Contrast") )
			contrast = paramXml->getIntAttribute("Contrast");
		if ( paramXml->hasAttribute("Exposure"))
			exposure = paramXml->getIntAttribute("Exposure");
		if ( paramXml->hasAttribute("LeftBorder") )
			displayMask->setEdge(BORDER::LEFT, paramXml->getIntAttribute("LeftBorder"));
		if ( paramXml->hasAttribute("RightBorder") )
			displayMask->setEdge(BORDER::RIGHT, paramXml->getIntAttribute("RightBorder"));
		if ( paramXml->hasAttribute("TopBorder") )
			displayMask->setEdge(BORDER::TOP, paramXml->getIntAttribute("TopBorder"));
		if ( paramXml->hasAttribute("BottomBorder") )
			displayMask->setEdge(BORDER::BOTTOM, paramXml->getIntAttribute("BottomBorder"));
		if ( paramXml->hasAttribute("AutoExposure") )
			auto_exposure = paramXml->getBoolAttribute("AutoExposure");
		if ( paramXml->hasAttribute("OverlayPath") )
			path_overlay = paramXml->getBoolAttribute("OverlayPath");
	}
	forEachXmlChildElementWithTagName(*parametersAsXml, deviceXml, "Devices")
	{
		if ( deviceXml->hasAttribute("Camera") )
			createNewCamera(deviceXml->getStringAttribute("Camera").toStdString());
		if ( deviceXml->hasAttribute("Format") )
		{
			if ( currentCam->ready() )
			{
				getDeviceList();
			}
			std::string fmt = deviceXml->getStringAttribute("Format").toStdString();
			setDeviceFormat(fmt);
		}
	}
	updateSettings();
}