#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>
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
		cv::Size board_size = cv::Size(m_width, m_height);
		std::vector<cv::Point2f> corners;
		for (int i = 0; i < imgs.size(); ++i)
		{
			cv::Mat grey;
			cv::cvtColor(imgs[i], grey, cv::COLOR_BGR2GRAY);
			if ( showImages ) {
				cv::imshow("grey", grey);
				cv::waitKey(1);
			}
			bool found = false;
			found = cv::findChessboardCorners(grey, board_size, corners, 1 | 4);
			if ( found ) {
				std::cout << "Got a chess board" << std::endl;
				cv::cornerSubPix(grey, corners, cv::Size(5,5), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1));
				cv::Mat col;
				grey.copyTo(col);
				cv::cvtColor(col, col, cv::COLOR_GRAY2BGR);
				cv::drawChessboardCorners(col, board_size, corners, found);
				if ( showImages ) {
					cv::imshow("grey", col);
					cv::waitKey(1);
				}
			}
		}
	};
private:
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
	 calibrator_A.reset();
	 calibrator_B.reset();
}

void StereoPos::updateSettings()
{}

AudioProcessorEditor * StereoPos::createEditor() {
	editor = new StereoPosEditor(this, true);
	return editor;

}

void StereoPos::showCapturedImages(bool show) {
	if ( show )
		cv::namedWindow("grey", cv::WINDOW_NORMAL);
	else
		cv::destroyWindow("grey");

}

void StereoPos::startStreaming() {
	// get some information abou the chessboard
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	double board_width = ed->getBoardDims(BOARDPROP::kWidth);
	double board_height = ed->getBoardDims(BOARDPROP::kHeight);
	double board_size = ed->getBoardDims(BOARDPROP::kSquareSize);
	calibrator_A = std::make_unique<CalibrateCamera>(board_width, board_height, board_size);
	calibrator_B = std::make_unique<CalibrateCamera>(board_width, board_height, board_size);

	GenericProcessor * maybe_merger = getSourceNode();
	if ( maybe_merger->isMerger() ) {
		auto ed = static_cast<StereoPosEditor*>(getEditor());
		double board_width = ed->getBoardDims(BOARDPROP::kWidth);
		double board_height = ed->getBoardDims(BOARDPROP::kHeight);
		double board_size = ed->getBoardDims(BOARDPROP::kSquareSize);
		maybe_merger->switchIO(0); // sourceNodeA
		video_A = (PosTracker*)maybe_merger->getSourceNode();
		if ( video_A ) {
			camera_A = video_A->getCurrentCamera();
			video_A->openCamera();
			video_A->getEditor()->updateSettings();
		}
		maybe_merger->switchIO(1); // sourceNodeA
		video_B = (PosTracker*)maybe_merger->getSourceNode();
		if ( video_B ) {
			camera_B = video_B->getCurrentCamera();
			video_B->openCamera();
			video_B->getEditor()->updateSettings();
		}
	}
	m_threadRunning = true;
	startThread(); // calls run
}

void StereoPos::stopStreaming() {
	if ( m_threadRunning ) {
		m_threadRunning = false;
		stopThread(10000);
		if ( video_A )
			video_A->stopCamera();
		if ( video_B )
			video_B->stopCamera();
	}
}

void StereoPos::run() {
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	bool showImages = ed->showCapturedImages();
	cv::Mat frame_A, frame_B;
	struct timeval tv;
	while ( m_threadRunning ) {
		if ( video_A ) {
			if ( video_A->isCamReady() ) {
				lock.enter();
				camera_A->read_frame(frame_A, tv);
				if ( ! frame_A.empty() ) {
					std::cout << "Calibrating " << video_A->getDeviceName() << "..." << std::endl;
					std::vector<cv::Mat> ims_A;
					ims_A.push_back(frame_A);
					calibrator_A->setup(ims_A, showImages);
				}
				lock.exit();
			}
		}
		if ( video_B ) {
			if ( video_B->isCamReady() ) {
				lock.enter();
				camera_B->read_frame(frame_B, tv);
				if ( ! frame_B.empty() ) {
					std::cout << "Calibrating " << video_B->getDeviceName() << "..." << std::endl;
					std::vector<cv::Mat> ims_B;
					ims_B.push_back(frame_B);
					calibrator_B->setup(ims_B, showImages);
				}
				lock.exit();
			}
		}
	}
}

void StereoPos::saveCustomParametersToXml(XmlElement * xml)
{
	xml->setAttribute("Type", "StereoPos");
	XmlElement * paramXml = xml->createNewChildElement("Parameters");
}

void StereoPos::loadCustomParametersFromXml()
{
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Parameters")
	{

	}
	updateSettings();
}
