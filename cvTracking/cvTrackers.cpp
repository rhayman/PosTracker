#include "cvTrackers.hpp"
#include "../common.h"

Font cvTracker::makeFont() {
	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
	MemoryInputStream mis(silk, silksize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	Font font = Font(typeface);
	font.setHeight(9);
	return font;
}

void cvTracker::addLabel(const std::string & name, const std::vector<int> & bounds) {
	auto font = makeFont();
	std::shared_ptr<Label> a = std::make_shared<Label>(name, name);
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setFont(font);
	a->setEditable (false, false, false);
	a->setJustificationType(Justification::centred);
	a->setColour (TextEditor::textColourId, Colours::grey);
	m_UILabels.push_back(std::move(a));
}

void cvTracker::addTextEditor(const std::string & name, const std::vector<int> & bounds, const std::string & initText="", const std::string & tooltip="") {
	std::shared_ptr<TextEditor> a = std::make_shared<TextEditor>(String(name));
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setText(initText);
	a->setTooltip(tooltip);
	m_UITextEditors.push_back(std::move(a));
	auto label_bounds = bounds;
	label_bounds[0] = label_bounds[0] - 55;
	addLabel(name, label_bounds);
}

void cvTracker::addCheckbox(const std::string & name, const std::vector<int> & bounds, const std::string & tooltip="") {
	std::shared_ptr<ToggleButton> a = std::make_shared<ToggleButton>("");
	a->setName(name);
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setToggleState(false, dontSendNotification);
	a->setTooltip(tooltip);
	m_UICheckBoxes.push_back(std::move(a));
	auto label_bounds = bounds;
	label_bounds[0] = label_bounds[0] - 55;
	addLabel(name, label_bounds);
}

void cvTracker::addComboBox(const std::string & name, const std::vector<int> & bounds, const std::vector<std::string> & entries, const std::string & tooltip="") {
	std::shared_ptr<ComboBox> a = std::make_shared<ComboBox>(String(name));
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setTooltip(tooltip);
	a->setEditableText(false);
	a->setJustificationType(Justification::centredLeft);
	for ( int i = 0; i < entries.size(); ++i )
		a->addItem(entries[i], i+1);
	m_UIComboBoxes.push_back(std::move(a));
	auto label_bounds = bounds;
	label_bounds[0] = label_bounds[0] - 55;
	addLabel(name, label_bounds);
}

void cvTracker::saveToXml(XmlElement * elem) {
	if ( ! m_UITextEditors.empty() ) {
		XmlElement * child = elem->createNewChildElement("TextEditorValues");
		for ( auto const & element : m_UITextEditors ) {
			auto name = element->getName();
			auto val = element->getText();
			XmlElement * elem = child->createNewChildElement(name);
			elem->setAttribute("Value", val);
		}
	}
	if ( ! m_UICheckBoxes.empty() ) {
		XmlElement * child = elem->createNewChildElement("CheckBoxValues");
		for ( auto const & element : m_UICheckBoxes ) {
			auto name = element->getName();
			auto val = element->getToggleState();
			XmlElement * elem = child->createNewChildElement(name);
			elem->setAttribute("Value", val);
		}
	}
	if ( ! m_UIComboBoxes.empty() ) {
		XmlElement * child = elem->createNewChildElement("ComboBoxValues");
		for ( auto const & element : m_UIComboBoxes ) {
			auto name = element->getName();
			auto val = element->getSelectedId();
			XmlElement * elem = child->createNewChildElement(name);
			elem->setAttribute("Value", val);
		}
	}
}

void cvTracker::setValue(const String & name, const String & val) {
	if ( ! m_UITextEditors.empty() ) {
		for ( auto & element : m_UITextEditors ) {
			auto this_name = element->getName();
			if ( this_name == name )
				element->setText(val);
		}
	}
	if ( ! m_UICheckBoxes.empty() ) {
		for ( auto & element : m_UICheckBoxes ) {
			auto this_name = element->getName();
			if ( this_name == name ) {
				if ( val == "0")
					element->setToggleState(false, dontSendNotification);
				else
					element->setToggleState(true, dontSendNotification);
			}
		}
	}
	if ( ! m_UIComboBoxes.empty() ) {
		for ( auto & element : m_UIComboBoxes ) {
			auto this_name = element->getName();
			if ( this_name == name )
				element->setSelectedId(val.getIntValue());
		}
	}
}

void BoostingTracker::makeTrackerUI() {

	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditor("initialIterations", te_bounds, "3", "The initial iterations");

	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditor("Classifiers", te1_bounds, "3", "The number of classifiers to use in a OnlineBoosting algorithm");

	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditor("samplerOverlap", te2_bounds, "2.0", "Search region parameters to use in a OnlineBoosting algorithm");

	std::vector<int> te3_bounds{145, 85, 50, 20};
	addTextEditor("samplerSearchFactor", te3_bounds, "2.0", "Search region parameters to use in a OnlineBoosting algorithm");
}

cv::Ptr<cv::Tracker> BoostingTracker::makeTracker() {
	auto params = cv::TrackerBoosting::Params();
	for ( auto const & element : m_UITextEditors ) {
		auto name = element->getName();
		if ( name == String("initialIterations") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.iterationInit = val;
		}
		if ( name == String("Classifiers") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.numClassifiers = val;
		}
		if ( name == String("samplerOverlap") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.samplerOverlap = val;
		}
		if ( name == String("samplerSearchFactor") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.samplerSearchFactor = val;
		}
	}
	return cv::TrackerBoosting::create(params);
}

void KCFTracker::makeTrackerUI() {
	std::vector<int> te_bounds{145, 25, 50, 20};
	addCheckbox("compressFeatures", te_bounds, "Activate the pca method to compress the features");

	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditor("compressedSize", te1_bounds, "2", "Feature size after compression");

	std::vector<int> te2_bounds{145, 65, 50, 20};
	// kcf_modes in common.h
	addComboBox("nonCompressedDesc", te2_bounds, kcf_modes, "non-compressed descriptors of KCF modes");

	std::vector<int> te3_bounds{145, 85, 50, 20};
	addComboBox("compressedDesc", te3_bounds, kcf_modes, "Compressed descriptors of KCF modes");

	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditor("detectionThreshold", te4_bounds, "0.5", "Detection confidence threshold");

	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditor("interpolationFactor", te5_bounds, "2", "Linear interpolation factor for adaptation");

	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditor("lambda", te6_bounds, "2", "Regularization");

	std::vector<int> te7_bounds{255, 65, 50, 20};
	addTextEditor("maxPatchSize", te7_bounds, "6400", "Threshold for the ROI size");

	std::vector<int> te8_bounds{255, 85, 50, 20};
	addTextEditor("outputSigmaFactor", te8_bounds, "0.0625", "Spatial bandwidth (proportional to target)");

	std::vector<int> te9_bounds{255, 105, 50, 20};
	addTextEditor("PCALearnRate", te9_bounds, "0.15", "Compression learning rate");

	std::vector<int> te10_bounds{365, 25, 50, 20};
	addCheckbox("resize", te10_bounds, "Activate the resize to speed up processing");

	std::vector<int> te11_bounds{365, 45, 50, 20};
	addTextEditor("Sigma", te11_bounds, "0.2", "Gaussian kernel bandwidth");

	std::vector<int> te12_bounds{365, 65, 50, 20};
	addCheckbox("splitCoeff", te12_bounds, "Split the training coefficients into two matrices");

	std::vector<int> te13_bounds{365, 85, 50, 20};
	addCheckbox("wrapKernel", te13_bounds, "Wrap around the kernel values");

}

cv::Ptr<cv::Tracker> KCFTracker::makeTracker() {
	auto params = cv::TrackerKCF::Params();
	for ( auto const & element : m_UITextEditors ) {
		auto name = element->getName();
		if ( name == String("compressedSize") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.compressed_size = val;
		}
		if ( name == String("detectionThreshold") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.detect_thresh = val;
		}
		if ( name == String("lambda") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.lambda = val;
		}
		if ( name == String("interpolationFactor") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.interp_factor = val;
		}
		if ( name == String("maxPatchSize") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.max_patch_size = val;
		}
		if ( name == String("outputSigmaFactor") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.output_sigma_factor = val;
		}
		if ( name == String("PCALearnRate") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.pca_learning_rate = val;
		}
		if ( name == String("Sigma") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.sigma = val;
		}
	}
	for ( auto const & element : m_UIComboBoxes ) {
		auto name = element->getName();
		if ( name == String("nonCompressedDesc") ) {
			ComboBox * t = static_cast<ComboBox*>(element.get());
			auto val = t->getSelectedId()-1;
			params.desc_npca = val;
		}
		if ( name == String("compressedDesc") ) {
			ComboBox * t = static_cast<ComboBox*>(element.get());
			auto val = t->getSelectedId()-1;
			params.desc_pca = val;
		}
	}
	for ( auto const & element : m_UICheckBoxes) {
		auto name = element->getName();
		if ( name == String("compressFeatures") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			params.compress_feature = val;
		}
	}
	for ( auto const & element : m_UICheckBoxes ) {
		auto name = element->getName();
		if ( name == String("resize") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			params.resize = val;
		}
		if ( name == String("splitCoeff") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			params.split_coeff = val;
		}
		if ( name == String("wrapKernel") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			params.wrap_kernel = val;
		}
	}
	return cv::TrackerKCF::create(params);
}

void MedianFlow::makeTrackerUI() {
	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditor("maxLevel", te_bounds, "10", "Maximal pyramid level number for Lucas-Kanade optical flow");

	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditor("maxDispLength", te1_bounds, "10", "Criterion for loosing the tracked object");

	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditor("pointsInGrid", te2_bounds, "10", "Square root of number of keypoints used; increase it to trade accurateness for speed");

	std::vector<int> te3_bounds{145, 85, 50, 20};
	std::vector<std::string> term_crit{"COUNT", "EPS", "COUNT|EPS"};
	addComboBox("termCrit", te3_bounds, term_crit, "Termination criteria for Lucas-Kanade optical flow");

	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditor("termCrit_maxCount", te4_bounds, "20", "Max count for Lucas-Kanade termination criteria");

	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditor("termCrit_EPS", te5_bounds, "0.3", "EPS for Lucas-Kanade termination criteria");

	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditor("windowSize", te6_bounds, "3", "Window size parameter for Lucas-Kanade optical flow");

	std::vector<int> te7_bounds{255, 65, 50, 20};
	addTextEditor("nccWindowSize", te7_bounds, "30", "Window size around a point for normalized cross-correlation check");

}

cv::Ptr<cv::Tracker> MedianFlow::makeTracker() {
	auto params = cv::TrackerMedianFlow::Params();
	auto t_crit = cv::TermCriteria();
	for ( auto const & element : m_UITextEditors ) {
		auto name = element->getName();
		if ( name == String("maxLevel") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.maxLevel = val;
		}
		if ( name == String("maxDispLength") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getDoubleValue();
			params.maxMedianLengthOfDisplacementDifference = val;
		}
		if ( name == String("pointsInGrid") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.pointsInGrid = val;
		}
		if ( name == String("termCrit_maxCount") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			t_crit.maxCount = val;
		}
		if ( name == String("termCrit_EPS") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			t_crit.epsilon = val;
		}
		if ( name == String("windowSize") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.winSize = cv::Size(val,val);
		}
		if ( name == String("nccWindowSize") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.winSizeNCC = cv::Size(val,val);
		}
	}
	for ( auto const & element : m_UIComboBoxes ) {
		auto name = element->getName();
		if ( name == String("termCrit") ) {
			ComboBox * t = static_cast<ComboBox*>(element.get());
			auto val = t->getSelectedId()-1;
			t_crit.type = val;
		}
	}
	params.termCriteria = t_crit;
	return cv::TrackerMedianFlow::create(params);
}

void MIL::makeTrackerUI() {
	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditor("initRadius", te_bounds, "3", "Maximal pyramid level number for Lucas-Kanade optical flow");

	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditor("searchWindowSize", te1_bounds, "25", "Criterion for loosing the tracked object");

	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditor("initMaxNegNumber", te2_bounds, "65", "Square root of number of keypoints used; increase it to trade accurateness for speed");

	std::vector<int> te3_bounds{145, 85, 50, 20};
	addTextEditor("trackInRadius", te3_bounds, "4", "Termination criteria for Lucas-Kanade optical flow");

	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditor("trackMaxPositiveNum", te4_bounds, "100000", "Max count for Lucas-Kanade termination criteria");

	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditor("trackMaxNegativeNum", te5_bounds, "65", "EPS for Lucas-Kanade termination criteria");

	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditor("featureSetNumFeatures", te6_bounds, "250", "Window size parameter for Lucas-Kanade optical flow");

}

cv::Ptr<cv::Tracker> MIL::makeTracker() {
	auto params = cv::TrackerMIL::Params();
	for ( auto const & element : m_UITextEditors ) {
		auto name = element->getName();
		if ( name == String("initRadius") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.samplerInitInRadius = val;
		}
		if ( name == String("searchWindowSize") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.samplerSearchWinSize = val;
		}
		if ( name == String("initMaxNegNumber") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.samplerInitMaxNegNum = val;
		}
		if ( name == String("trackInRadius") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.samplerTrackInRadius = val;
		}
		if ( name == String("trackMaxPositiveNum") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.samplerTrackMaxPosNum = val;
		}
		if ( name == String("trackMaxNegativeNum") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.samplerTrackMaxNegNum = val;
		}
		if ( name == String("featureSetNumFeatures") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.featureSetNumFeatures = val;
		}
	}
	return cv::TrackerMIL::create(params);
}

void Background::makeTrackerUI() {
	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditor("backgroundRatio", te_bounds, "1", "If a foreground pixel keeps semi-constant \
																	value for about backgroundRatio*history frames, \
																	it's considered background and added to the model \
																	as a center of a new component. \
																	It corresponds to TB parameter in the paper.");

	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditor("complexityReductionThresh", te1_bounds, "0.05", "This parameter defines the number of samples needed to accept to\
																				 prove the component exists. CT=0.05 is a default value for all the samples. \
																				By setting CT=0 you get an algorithm very similar to the standard Stauffer&Grimson algorithm.");
	std::vector<int> te2_bounds{145, 65, 50, 20};
	addCheckbox("detectShadows", te2_bounds, "Enables or disables shadow detection");

	std::vector<int> te3_bounds{145, 85, 50, 20};
	addTextEditor("setHistory", te3_bounds, "500", "the number of frames that affect the background model");

	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditor("nMixtures", te4_bounds, "5", "The number of gaussian components in the model");

	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditor("shadowThreshold", te5_bounds, "0.5", "A shadow is detected if pixel is a darker version of the background.\
													 The shadow threshold (Tau in the paper) is a threshold defining how much darker the \
													 shadow can be. Tau= 0.5 means that if a pixel is more than twice darker then it is not shadow");
	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditor("shadowValue", te6_bounds, "127", "Shadow value is the value used to mark shadows in the foreground mask.\
															 Default value is 127. Value 0 in the mask always means background, 255 means foreground");
	std::vector<int> te7_bounds{255, 65, 50, 20};
	addTextEditor("initialVariance", te7_bounds, "15", "The initial variance of each gaussian component.");

	std::vector<int> te8_bounds{255, 85, 50, 20};
	addTextEditor("varianceMin", te8_bounds, "4", "The minimum variance");

	std::vector<int> te9_bounds{255, 105, 50, 20};
	addTextEditor("varianceMax", te9_bounds, "75", "The maximum variance");

	std::vector<int> te10_bounds{365, 25, 50, 20};
	addTextEditor("varianceThresh", te10_bounds, "16", "The variance threshold for the pixel-model match.\
															The main threshold on the squared Mahalanobis distance to decide if the\
															 sample is well described by the background model or not. Related to Cthr from the paper.");

	std::vector<int> te11_bounds{365, 45, 50, 20};
	addTextEditor("varianceThreshGen", te11_bounds, "9", "The variance threshold for the pixel-model match used for new mixture component generation.\
																		Threshold for the squared Mahalanobis distance that helps decide when a sample is close to the\
																		 existing components (corresponds to Tg in the paper). If a pixel is not close to any component,\
																		  it is considered foreground or added as a new component. 3 sigma => Tg=3*3=9 is default.\
																		  A smaller Tg value generates more components. A higher Tg value may result in a small number\
																		   of components but they can grow too large.");
}

cv::Ptr<cv::BackgroundSubtractor> Background::makeBackgroundSubtractor() {
	cv::Ptr<cv::BackgroundSubtractorMOG2> background_sub = cv::createBackgroundSubtractorMOG2();
	for ( auto const & element : m_UITextEditors ) {
		auto name = element->getName();
		if ( name == String("backgroundRatio") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setBackgroundRatio(val);
		}
		if ( name == String("complexityReductionThresh") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setComplexityReductionThreshold(val);
		}
		if ( name == String("setHistory") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setHistory(val);
		}
		if ( name == String("nMixtures") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setNMixtures(val);
		}
		if ( name == String("shadowThreshold") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setShadowThreshold(val);
		}
		if ( name == String("shadowValue") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setShadowValue(val);
		}
		if ( name == String("initialVariance") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setVarInit(val);
		}
		if ( name == String("varianceMin") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setVarMin(val);
		}
		if ( name == String("varianceMax") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setVarMax(val);
		}
		if ( name == String("varianceThresh") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setVarThreshold(val);
		}
		if ( name == String("varianceThreshGen") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setVarThresholdGen(val);
		}
	}
	for ( auto const & element : m_UICheckBoxes ) {
		auto name = element->getName();
		if ( name == String("detectShadows") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			background_sub->setDetectShadows(val);
		}
	}
	return background_sub;
}

cv::Ptr<cv::Tracker> Background::makeTracker() {
	auto params = cv::TrackerMIL::Params();
	return cv::TrackerMIL::create(params);
}

void BackgroundKNN::makeTrackerUI() {
	std::vector<int> te_bounds{145, 25, 50, 20};
	addCheckbox("detectShadows", te_bounds, "Enable/ disable shadow detection");

	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditor("dist2Thresh", te1_bounds, "400", "The threshold on the squared distance between the pixel and the sample. The threshold on the squared distance between the pixel and the sample to decide whether a pixel is close to a data sample.");

	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditor("setHistory", te2_bounds, "500", "The number of last frames that affect the background model");

	std::vector<int> te3_bounds{145, 85, 50, 20};
	addTextEditor("setKNNSamples", te3_bounds, "2", "the number of frames that affect the background model");

	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditor("setNSamples", te4_bounds, "7", "The number of gaussian components in the model");

	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditor("shadowThreshold", te5_bounds, "0.5", "A shadow is detected if pixel is a darker version of the background. The shadow threshold (Tau in the paper) is a threshold defining how much darker the shadow can be. Tau= 0.5 means that if a pixel is more than twice darker then it is not shadow");

	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditor("shadowValue", te6_bounds, "127", "Shadow value is the value used to mark shadows in the foreground mask. Default value is 127. Value 0 in the mask always means background, 255 means foreground");
}

cv::Ptr<cv::Tracker> BackgroundKNN::makeTracker() {
	auto params = cv::TrackerMIL::Params();
	return cv::TrackerMIL::create(params);
}

cv::Ptr<cv::BackgroundSubtractor> BackgroundKNN::makeBackgroundSubtractor() {
	cv::Ptr<cv::BackgroundSubtractorKNN> background_sub = cv::createBackgroundSubtractorKNN();
	for ( auto const & element : m_UITextEditors ) {
		auto name = element->getName();
		if ( name == String("dist2Thresh") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setDist2Threshold(val);
		}
		if ( name == String("setHistory") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setHistory(val);
		}
		if ( name == String("setKNNSamples") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setkNNSamples(val);
		}
		if ( name == String("setNSamples") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setNSamples(val);
		}
		if ( name == String("shadowThreshold") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setShadowThreshold(val);
		}
		if ( name == String("shadowValue") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			background_sub->setShadowValue(val);
		}
	}
	for ( auto const & element : m_UICheckBoxes ) {
		auto name = element->getName();
		if ( name == String("detectShadows") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			background_sub->setDetectShadows(val);
		}
	}
	return background_sub;
}