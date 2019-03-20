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

void cvTracker::addLabelToUIElements(const std::string & name, const std::vector<int> & bounds) {
	auto font = makeFont();
	std::shared_ptr<Label> a = std::make_shared<Label>(name, name);
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setFont(font);
	a->setEditable (false, false, false);
	a->setJustificationType(Justification::centred);
	a->setColour (TextEditor::textColourId, Colours::grey);
	m_UIElements.push_back(std::move(a));
}

void cvTracker::addTextEditorToUIElements(const std::string & name, const std::vector<int> & bounds, const std::string & initText="", const std::string & tooltip="") {
	std::shared_ptr<TextEditor> a = std::make_shared<TextEditor>(String(name));
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setText(initText);
	a->setTooltip(tooltip);
	m_UIElements.push_back(std::move(a));
}

void cvTracker::addCheckboxTouiElements(const std::string & name, const std::vector<int> & bounds, const std::string & tooltip="") {
	std::shared_ptr<ToggleButton> a = std::make_shared<ToggleButton>("");
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setToggleState(false, dontSendNotification);
	a->setTooltip(tooltip);
	m_UIElements.push_back(std::move(a));
}

void cvTracker::addComboBoxToUIElements(const std::string & name, const std::vector<int> & bounds, const std::vector<std::string> & entries, const std::string & tooltip="") {
	std::shared_ptr<ComboBox> a = std::make_shared<ComboBox>(String(name));
	a->setBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	a->setTooltip(tooltip);
	a->setEditableText(false);
	a->setJustificationType(Justification::centredLeft);
	for ( int i = 0; i < entries.size(); ++i )
		a->addItem(entries[i], i+1);
	m_UIElements.push_back(std::move(a));
}

void BoostingTracker::makeTrackerUI() {

	std::vector<int> label_bounds{90, 25, 50, 20};
	addLabelToUIElements("Iterations", label_bounds);
	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditorToUIElements("initialIterations", te_bounds, "3", "The initial iterations");

	std::vector<int> label1_bounds{90, 45, 50, 20};
	addLabelToUIElements("# Classifiers", label1_bounds);
	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditorToUIElements("Classifiers", te1_bounds, "3", "The number of classifiers to use in a OnlineBoosting algorithm");

	std::vector<int> label2_bounds{90, 65, 50, 20};
	addLabelToUIElements("Overlap", label2_bounds);
	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditorToUIElements("samplerOverlap", te2_bounds, "2.0", "Search region parameters to use in a OnlineBoosting algorithm");

	std::vector<int> label3_bounds{90, 85, 50, 20};
	addLabelToUIElements("Search", label3_bounds);
	std::vector<int> te3_bounds{145, 85, 50, 20};
	addTextEditorToUIElements("samplerSearchFactor", te3_bounds, "2.0", "Search region parameters to use in a OnlineBoosting algorithm");

}

cv::Ptr<cv::Tracker> BoostingTracker::makeTracker() {
	auto params = cv::TrackerBoosting::Params();
	for ( auto & element : m_UIElements ) {
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
	std::vector<int> label_bounds{90, 25, 50, 20};
	addLabelToUIElements("Compress features", label_bounds);
	std::vector<int> te_bounds{145, 25, 50, 20};
	addCheckboxTouiElements("compressFeatures", te_bounds, "Activate the pca method to compress the features");

	std::vector<int> label1_bounds{90, 45, 50, 20};
	addLabelToUIElements("Compressed size", label1_bounds);
	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditorToUIElements("compressedSize", te1_bounds, "2", "Feature size after compression");

	std::vector<int> label2_bounds{90, 65, 50, 20};
	addLabelToUIElements("Non-compressed descriptors", label2_bounds);
	std::vector<int> te2_bounds{145, 65, 50, 20};
	// kcf_modes in common.h
	addComboBoxToUIElements("nonCompressedDesc", te2_bounds, kcf_modes, "non-compressed descriptors of KCF modes");

	std::vector<int> label3_bounds{90, 85, 50, 20};
	addLabelToUIElements("Compressed descriptors", label3_bounds);
	std::vector<int> te3_bounds{145, 85, 50, 20};
	addComboBoxToUIElements("compressedDesc", te3_bounds, kcf_modes, "Compressed descriptors of KCF modes");

	std::vector<int> label4_bounds{90, 105, 50, 20};
	addLabelToUIElements("Detection threshold", label4_bounds);
	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditorToUIElements("detectionThreshold", te4_bounds, "0.5", "Detection confidence threshold");

	std::vector<int> label5_bounds{200, 25, 50, 20};
	addLabelToUIElements("Interpolation factor", label5_bounds);
	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditorToUIElements("interpolationFactor", te5_bounds, "2", "Linear interpolation factor for adaptation");

	std::vector<int> label6_bounds{200, 45, 50, 20};
	addLabelToUIElements("Lambda", label6_bounds);
	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditorToUIElements("lambda", te6_bounds, "2", "Regularization");

	std::vector<int> label7_bounds{200, 65, 50, 20};
	addLabelToUIElements("Max patch size", label7_bounds);
	std::vector<int> te7_bounds{255, 65, 50, 20};
	addTextEditorToUIElements("maxPatchSize", te7_bounds, "6400", "Threshold for the ROI size");

	std::vector<int> label8_bounds{200, 85, 50, 20};
	addLabelToUIElements("Ouput sigma", label8_bounds);
	std::vector<int> te8_bounds{255, 85, 50, 20};
	addTextEditorToUIElements("outputSigmaFactor", te8_bounds, "0.0625", "Spatial bandwidth (proportional to target)");

	std::vector<int> label9_bounds{200, 105, 50, 20};
	addLabelToUIElements("PCA learn rate", label9_bounds);
	std::vector<int> te9_bounds{255, 105, 50, 20};
	addTextEditorToUIElements("PCALearnRate", te9_bounds, "0.15", "Compression learning rate");

	std::vector<int> label10_bounds{310, 25, 50, 20};
	addLabelToUIElements("Resize", label10_bounds);
	std::vector<int> te10_bounds{365, 25, 50, 20};
	addCheckboxTouiElements("resize", te10_bounds, "Activate the resize to speed up processing");

	std::vector<int> label11_bounds{310, 45, 50, 20};
	addLabelToUIElements("Sigma", label11_bounds);
	std::vector<int> te11_bounds{365, 45, 50, 20};
	addTextEditorToUIElements("Sigma", te11_bounds, "0.2", "Gaussian kernel bandwidth");

	std::vector<int> label12_bounds{310, 65, 50, 20};
	addLabelToUIElements("Splitt coeff", label12_bounds);
	std::vector<int> te12_bounds{365, 65, 50, 20};
	addCheckboxTouiElements("splitCoeff", te12_bounds, "Split the training coefficients into two matrices");

	std::vector<int> label13_bounds{310, 85, 50, 20};
	addLabelToUIElements("Wrap kernel", label13_bounds);
	std::vector<int> te13_bounds{365, 85, 50, 20};
	addCheckboxTouiElements("wrapKernel", te13_bounds, "Wrap around the kernel values");

}

cv::Ptr<cv::Tracker> KCFTracker::makeTracker() {
	auto params = cv::TrackerKCF::Params();
	for ( auto & element : m_UIElements ) {
		auto name = element->getName();
		if ( name == String("compressFeatures") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			params.compress_feature = val;
		}
		if ( name == String("compressedSize") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getIntValue();
			params.compressed_size = val;
		}
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
		if ( name == String("detectionThreshold") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.detect_thresh = val;
		}
		if ( name == String("interpolationFactor") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.interp_factor = val;
		}
		if ( name == String("lambda") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.lambda = val;
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
		if ( name == String("resize") ) {
			ToggleButton * t = static_cast<ToggleButton*>(element.get());
			auto val = t->getToggleState();
			params.resize = val;
		}
		if ( name == String("Sigma") ) {
			TextEditor * t = static_cast<TextEditor*>(element.get());
			auto val = t->getText().getFloatValue();
			params.sigma = val;
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
	std::vector<int> label_bounds{90, 25, 50, 20};
	addLabelToUIElements("Max level", label_bounds);
	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditorToUIElements("maxLevel", te_bounds, "10", "Maximal pyramid level number for Lucas-Kanade optical flow");

	std::vector<int> label1_bounds{90, 45, 50, 20};
	addLabelToUIElements("Max displacement length", label1_bounds);
	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditorToUIElements("maxDispLength", te1_bounds, "10", "Criterion for loosing the tracked object");

	std::vector<int> label2_bounds{90, 65, 50, 20};
	addLabelToUIElements("Points in grid", label2_bounds);
	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditorToUIElements("pointsInGrid", te2_bounds, "10", "Square root of number of keypoints used; increase it to trade accurateness for speed");

	std::vector<int> label3_bounds{90, 85, 50, 20};
	addLabelToUIElements("Termination criteria", label3_bounds);
	std::vector<int> te3_bounds{145, 85, 50, 20};
	std::vector<std::string> term_crit{"COUNT", "EPS", "COUNT|EPS"};
	addComboBoxToUIElements("termCrit", te3_bounds, term_crit, "Termination criteria for Lucas-Kanade optical flow");

	std::vector<int> label4_bounds{90, 105, 50, 20};
	addLabelToUIElements("Max count", label4_bounds);
	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditorToUIElements("termCrit_maxCount", te4_bounds, "20", "Max count for Lucas-Kanade termination criteria");

	std::vector<int> label5_bounds{200, 25, 50, 20};
	addLabelToUIElements("EPS", label5_bounds);
	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditorToUIElements("termCrit_EPS", te5_bounds, "0.3", "EPS for Lucas-Kanade termination criteria");

	std::vector<int> label6_bounds{200, 45, 50, 20};
	addLabelToUIElements("Window size", label6_bounds);
	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditorToUIElements("windowSize", te6_bounds, "3", "Window size parameter for Lucas-Kanade optical flow");

	std::vector<int> label7_bounds{200, 65, 50, 20};
	addLabelToUIElements("NCC window size", label7_bounds);
	std::vector<int> te7_bounds{255, 65, 50, 20};
	addTextEditorToUIElements("nccWindowSize", te7_bounds, "30", "Window size around a point for normalized cross-correlation check");

}

cv::Ptr<cv::Tracker> MedianFlow::makeTracker() {
	auto params = cv::TrackerMedianFlow::Params();
	auto t_crit = cv::TermCriteria();
	for ( auto & element : m_UIElements ) {
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
		if ( name == String("termCrit") ) {
			ComboBox * t = static_cast<ComboBox*>(element.get());
			auto val = t->getSelectedId()-1;
			t_crit.type = val;
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
		params.termCriteria = t_crit;
	}
	return cv::TrackerMedianFlow::create(params);
}

void MIL::makeTrackerUI() {
	std::vector<int> label_bounds{90, 25, 50, 20};
	addLabelToUIElements("Init radius", label_bounds);
	std::vector<int> te_bounds{145, 25, 50, 20};
	addTextEditorToUIElements("initRadius", te_bounds, "3", "Maximal pyramid level number for Lucas-Kanade optical flow");

	std::vector<int> label1_bounds{90, 45, 50, 20};
	addLabelToUIElements("Search window size", label1_bounds);
	std::vector<int> te1_bounds{145, 45, 50, 20};
	addTextEditorToUIElements("searchWindowSize", te1_bounds, "25", "Criterion for loosing the tracked object");

	std::vector<int> label2_bounds{90, 65, 50, 20};
	addLabelToUIElements("Initial max neg num", label2_bounds);
	std::vector<int> te2_bounds{145, 65, 50, 20};
	addTextEditorToUIElements("initMaxNegNumber", te2_bounds, "65", "Square root of number of keypoints used; increase it to trade accurateness for speed");

	std::vector<int> label3_bounds{90, 85, 50, 20};
	addLabelToUIElements("Track in radius", label3_bounds);
	std::vector<int> te3_bounds{145, 85, 50, 20};
	addTextEditorToUIElements("trackInRadius", te3_bounds, "4", "Termination criteria for Lucas-Kanade optical flow");

	std::vector<int> label4_bounds{90, 105, 50, 20};
	addLabelToUIElements("Track max pos num", label4_bounds);
	std::vector<int> te4_bounds{145, 105, 50, 20};
	addTextEditorToUIElements("trackMaxPositiveNum", te4_bounds, "100000", "Max count for Lucas-Kanade termination criteria");

	std::vector<int> label5_bounds{200, 25, 50, 20};
	addLabelToUIElements("Track max neg num", label5_bounds);
	std::vector<int> te5_bounds{255, 25, 50, 20};
	addTextEditorToUIElements("trackMaxNegativeNum", te5_bounds, "65", "EPS for Lucas-Kanade termination criteria");

	std::vector<int> label6_bounds{200, 45, 50, 20};
	addLabelToUIElements("Feature set num features", label6_bounds);
	std::vector<int> te6_bounds{255, 45, 50, 20};
	addTextEditorToUIElements("featureSetNumFeatures", te6_bounds, "250", "Window size parameter for Lucas-Kanade optical flow");

}

cv::Ptr<cv::Tracker> MIL::makeTracker() {
	auto params = cv::TrackerMIL::Params();
	for ( auto & element : m_UIElements ) {
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