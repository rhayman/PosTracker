#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <unistd.h>
#include <cuchar>
#include <ctime>
#include "StereoPos.h"
#include "StereoPosEditor.h"
#include "../PosTracker/PosTracker.h"
#include "../PosTracker/Camera.h"

class CalibrateCamera {
public:
	CalibrateCamera() {};
	CalibrateCamera(double width, double height, double size) : m_width(width), m_height(height), m_size(size) {};
	~CalibrateCamera() {};
	void setup(std::vector<cv::Mat> imgs, bool showImages=false) {
		m_showImages = showImages;
		cv::Size board_size = cv::Size(m_width, m_height);
		std::vector<cv::Point2f> corners;
		cv::SimpleBlobDetector::Params params;
		params.minArea = 10;
		params.maxArea = 10e5;
		params.minDistBetweenBlobs = 5;
		cv::Ptr<cv::SimpleBlobDetector> blobDetector = cv::SimpleBlobDetector::create(params);
		for (int i = 0; i < imgs.size(); ++i)
		{
			cv::Mat grey;
			cv::cvtColor(imgs[i], grey, cv::COLOR_BGR2GRAY);
			m_found = cv::findCirclesGrid(grey, board_size, corners, cv::CALIB_CB_SYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, blobDetector);
			if ( m_found ) {
				std::cout << "Got a chess board" << std::endl;
				cv::cornerSubPix(grey, corners, cv::Size(5,5), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1));
				cv::Mat col;
				grey.copyTo(col);
				cv::cvtColor(col, col, cv::COLOR_GRAY2BGR);
				cv::drawChessboardCorners(col, board_size, corners, m_found);
				if ( showImages ) {
					cv::imwrite("/home/robin/tmp/imgs/chessboard_" + std::to_string(i) + ".png", col);
				}
			}
		}
	};
private:
	bool m_found = false;
	bool m_showImages = false;
	double m_width = 11;
	double m_height = 12;
	double m_size = 11;
};

StereoPos::StereoPos() : GenericProcessor("Stereo Pos"), Thread("StereoPosThread")
{
	setProcessorType (PROCESSOR_TYPE_FILTER);
	sendSampleCount = false;
}

StereoPos::~StereoPos() {
	 for (int i = 0; i < calibrators.size(); ++i)
	 {
	 	calibrators[i].reset();
	 }
	 cv::destroyAllWindows();
}

void StereoPos::updateSettings()
{
	if ( editor != NULL )
		editor->updateSettings();
}

AudioProcessorEditor * StereoPos::createEditor() {
	editor = new StereoPosEditor(this, true);
	return editor;

}

void StereoPos::showCapturedImages(bool show) {
	// if ( show ) {
	// 	if ( ! m_trackers.empty() ) {
	// 		for (int i = 0; i < m_trackers.size(); ++i) {
	// 			cv::namedWindow("capture_" + std::to_string(i));
	// 		}
	// 	}
	// }
	// else
	// 	if ( ! m_trackers.empty() ) {
	// 		for (int i = 0; i < m_trackers.size(); ++i) {
	// 			cv::destroyWindow("capture_" + std::to_string(i));
	// 		}
	// 	}
}

void StereoPos::startStreaming() {
	// get some information abou the chessboard
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	nImagesToCapture = ed->getNImagesToCapture();
	double board_width = ed->getBoardDims(BOARDPROP::kWidth);
	double board_height = ed->getBoardDims(BOARDPROP::kHeight);
	double board_size = ed->getBoardDims(BOARDPROP::kSquareSize);
	std::cout << "Looking for a chessboard " << board_width << " corners wide, " << board_height << " corners high\n";
	GenericProcessor * maybe_merger = getSourceNode();
	m_trackers.clear();
	bool showims = ed->showCapturedImages();
	showCapturedImages(showims);
	if ( maybe_merger->isMerger() ) {
		maybe_merger->switchIO(0); // sourceNodeA
		PosTracker * tracker = (PosTracker*)maybe_merger->getSourceNode();
		if ( tracker != nullptr ) {
			tracker = (PosTracker*)maybe_merger->getSourceNode();
			tracker->openCamera();
			tracker->getEditor()->updateSettings();
			m_trackers.push_back(tracker);
			calibrators.push_back(std::make_unique<CalibrateCamera>(board_width, board_height, board_size));
		}
		maybe_merger->switchIO(1); // sourceNodeA
		tracker = (PosTracker*)maybe_merger->getSourceNode();
		if ( tracker != nullptr ) {
			tracker = (PosTracker*)maybe_merger->getSourceNode();
			tracker->openCamera();
			tracker->getEditor()->updateSettings();
			m_trackers.push_back(tracker);
			calibrators.push_back(std::make_unique<CalibrateCamera>(board_width, board_height, board_size));
		}
	}

	m_threadRunning = true;
	startThread(); // calls run
}

void StereoPos::stopStreaming() {
	if ( m_threadRunning ) {
		m_threadRunning = false;
		stopThread(1000);
		for (int i = 0; i < m_trackers.size(); ++i) {
			m_trackers[i]->stopCamera();
		}
	}
}

void StereoPos::run() {
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	int pauseBetweenCapsSecs = ed->getNSecondsBetweenCaptures();
	bool showImages = ed->showCapturedImages();
	cv::Mat frame;
	unsigned int count = 0;
	bool doCapture = false;
	// containers for various parts of the opencv calibration algos
	std::vector<std::vector<cv::Mat>> images{m_trackers.size()};
	std::time_t starttime = std::time(nullptr);
	while ( ( count < (nImagesToCapture * m_trackers.size()) ) && m_threadRunning ) {
		std::time_t nowtime = std::time(nullptr);
		for (int i = 0; i < m_trackers.size(); ++i) {
			PosTracker * tracker = m_trackers[i];
			if ( tracker->isCamReady() ) {
				if ( std::difftime(nowtime, starttime) > pauseBetweenCapsSecs) {
					if ( tracker->isStreaming() ) {
						std::cout << "Saving images after " << std::difftime(nowtime, starttime) << " seconds" << std::endl;
						Formats * currentFmt = tracker->getCurrentFormat();
						frame = cv::Mat(currentFmt->height, currentFmt->width, CV_8UC3, (unsigned char*)tracker->get_frame_ptr());
						cv::Mat frame_clone = frame.clone();
						images[i].push_back(frame_clone);
						cv::imwrite("/home/robin/tmp/imgs/capture_" + std::to_string(count) + ".png", frame_clone);
						++count;
					}
				}
			}
		}
		std::time_t starttime = std::time(nullptr);
		sleep(pauseBetweenCapsSecs*1000);
	}
	for (int i = 0; i < m_trackers.size(); ++i) {
		std::cout << "Calibrating camera  " << i << std::endl;
		calibrators[i]->setup(images[i], showImages);
	}
}

void StereoPos::saveCustomParametersToXml(XmlElement * xml)
{
	xml->setAttribute("Type", "StereoPos");
	XmlElement * paramXml = xml->createNewChildElement("Parameters");
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	paramXml->setAttribute("numImages", ed->getNImagesToCapture());
	paramXml->setAttribute("timeDelay", ed->getNSecondsBetweenCaptures());
	paramXml->setAttribute("numColumns", ed->getBoardDims(BOARDPROP::kWidth));
	paramXml->setAttribute("numRows", ed->getBoardDims(BOARDPROP::kHeight));
	paramXml->setAttribute("squareSize", ed->getBoardDims(BOARDPROP::kSquareSize));
}

void StereoPos::loadCustomParametersFromXml()
{
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Parameters")
	{

	}
	updateSettings();
}
