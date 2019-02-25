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
	~CalibrateCamera() {
		if ( m_showImages && m_found )
			cv::destroyWindow("Chessboard detection");
	};
	void setup(std::vector<cv::Mat> imgs, bool showImages=false) {
		m_showImages = showImages;
		cv::Size board_size = cv::Size(m_width, m_height);
		std::vector<cv::Point2f> corners;
		for (int i = 0; i < imgs.size(); ++i)
		{
			cv::Mat grey;
			cv::cvtColor(imgs[i], grey, cv::COLOR_BGR2GRAY);
			m_found = cv::findChessboardCorners(grey, board_size, corners, 1 | 4);
			if ( m_found ) {
				std::cout << "Got a chess board" << std::endl;
				cv::cornerSubPix(grey, corners, cv::Size(5,5), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1));
				cv::Mat col;
				grey.copyTo(col);
				cv::cvtColor(col, col, cv::COLOR_GRAY2BGR);
				cv::drawChessboardCorners(col, board_size, corners, m_found);
				if ( showImages ) {
					cv::namedWindow("Chessboard detection");
					cv::imshow("Chessboard detection", col);
					cv::waitKey(1);
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
	 calibrator_A.reset();
	 calibrator_B.reset();
	 cv::destroyAllWindows();
}

void StereoPos::updateSettings()
{}

AudioProcessorEditor * StereoPos::createEditor() {
	editor = new StereoPosEditor(this, true);
	return editor;

}

void StereoPos::showCapturedImages(bool show) {
	if ( show ) {
		cv::namedWindow("Camera A", cv::WINDOW_NORMAL);
		cv::namedWindow("Camera B", cv::WINDOW_NORMAL);
	}
	else {
		cv::destroyWindow("Camera A");
		cv::destroyWindow("Camera B");
	}

}

void StereoPos::startStreaming() {
	// get some information abou the chessboard
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	nImagesToCapture = ed->getNImagesToCapture();
	double board_width = ed->getBoardDims(BOARDPROP::kWidth);
	double board_height = ed->getBoardDims(BOARDPROP::kHeight);
	double board_size = ed->getBoardDims(BOARDPROP::kSquareSize);
	calibrator_A = std::make_unique<CalibrateCamera>(board_width, board_height, board_size);
	calibrator_B = std::make_unique<CalibrateCamera>(board_width, board_height, board_size);
	GenericProcessor * maybe_merger = getSourceNode();
	if ( maybe_merger->isMerger() ) {
		maybe_merger->switchIO(0); // sourceNodeA
		video_A = (PosTracker*)maybe_merger->getSourceNode();
		if ( video_A->isEnabled ) {
			video_A = (PosTracker*)maybe_merger->getSourceNode();
			video_A->openCamera();
			video_A->getEditor()->updateSettings();
			camera_A = video_A->getCurrentCamera();
		}
		else
			video_A = nullptr;
		maybe_merger->switchIO(1); // sourceNodeA
		video_B = (PosTracker*)maybe_merger->getSourceNode();
		if ( video_B->isEnabled ) {
			video_B->openCamera();
			video_B->getEditor()->updateSettings();
			camera_B = video_B->getCurrentCamera();
		}
		else
			video_B = nullptr;
	}

	m_threadRunning = true;
	startThread(); // calls run
}

void StereoPos::stopStreaming() {
	if ( m_threadRunning ) {
		m_threadRunning = false;
		stopThread(1000);
		if ( video_A )
			video_A->stopCamera();
		if ( video_B )
			video_B->stopCamera();
	}
}

void StereoPos::run() {
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	int pauseBetweenCapsSecs = ed->getNSecondsBetweenCaptures();
	bool showImages = ed->showCapturedImages();
	cv::Mat frame_A, frame_B;
	struct timeval tv;
	unsigned int count = 0;
	// containers for various parts of the opencv calibration algos
	std::vector<cv::Mat> ims_A;
	std::vector<cv::Mat> ims_B;
	while ( (count <= nImagesToCapture) && m_threadRunning ) {
		if ( video_A ) {
			if ( video_A->isCamReady() ) {
				std::cout << "capturing on A\n";
				camera_A->read_frame(frame_A, tv);
				if ( ! frame_A.empty() ) {
					ims_A.push_back(frame_A);
					if ( showImages ) {
						cv::imshow("Camera A", frame_A);
						cv::waitKey(1);
					}

				}
			}
		}
		if ( video_B ) {
			if ( video_B->isCamReady() ) {
				std::cout << "capturing on A\n";
				camera_B->read_frame(frame_B, tv);
				if ( ! frame_B.empty() ) {
					ims_B.push_back(frame_B);
					if ( showImages ) {
						cv::imshow("Camera B", frame_B);
						cv::waitKey(1);
					}
				}
			}
		}
		sleep(pauseBetweenCapsSecs*1000);
		++count;
	}
	if ( video_A ) {
		std::cout << "Calibrating camera A with " << ims_A.size() << " images" << std::endl;
		calibrator_A->setup(ims_A, showImages);
	}
	if ( video_B ) {
		std::cout << "Calibrating camera B with " << ims_B.size() << " images" << std::endl;
		calibrator_B->setup(ims_B, showImages);
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
