#include "PosTrackerEditor.h"
#include "PosTracker.h"

PosTrackerEditor::PosTrackerEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors=true)
: GenericEditor(parentNode, useDefaultParameterEditors)
{
	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
	MemoryInputStream mis(silk, silksize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	Font font = Font(typeface);

	desiredWidth = 495;
	m_proc = (PosTracker*)getProcessor();

	// Video source stuff
	sourceCombo = std::make_unique<ComboBox>("sourceCombo");
	sourceCombo->setBounds(5, 30, 120, 20); // x,y,w,h
	sourceCombo->setTooltip("The video device (e.g. /dev/video0)");
	sourceCombo->setEditableText(false);
	sourceCombo->setJustificationType(Justification::centredLeft);
	sourceCombo->setTextWhenNothingSelected("Device");
	sourceCombo->setTextWhenNoChoicesAvailable("Device");
	addAndMakeVisible(sourceCombo.get());

	auto video_devices = m_proc->getDeviceList();
	for (int i = 0; i < video_devices.size(); ++i)
		sourceCombo->addItem(video_devices.at(i), i+1);

	sourceCombo->addListener(this);

	resolution = std::make_unique<ComboBox>("resolutionMenu");
	resolution->setBounds(5, 55, 120, 20);
	resolution->setTooltip (("The size and frame rate of the video output (e.g. 640 x 480, 30fps)"));
	resolution->setEditableText (false);
	resolution->setJustificationType (Justification::centredLeft);
	resolution->setTextWhenNothingSelected (("Resolution/ fps"));
	resolution->setTextWhenNoChoicesAvailable (("Resolution / fps"));
	resolution->addListener(this);
	addAndMakeVisible(resolution.get());

	refreshBtn = std::make_unique<UtilityButton>("Refresh", Font ("Small Text", 10, Font::plain));
	refreshBtn->setBounds(5, 80, 60, 20);
	refreshBtn->setTooltip("Refresh the list of devices");
	refreshBtn->addListener(this);
	addAndMakeVisible(refreshBtn.get());

	showVideo = std::make_unique<ToggleButton>("Show video");
	showVideo->setBounds(5, 105, 100, 20);
	showVideo->setTooltip("Show the video stream");
	showVideo->addListener(this);
	showVideo->setEnabled(false);
	addAndMakeVisible(showVideo.get());

	font.setHeight(8);

	// autoexposure button
	autoExposure = std::make_unique<ToggleButton>("Auto exposure");
	autoExposure->setBounds(345, 72, 60, 40); //  smaller than this and the JUCE graphics_context goes mental
	autoExposure->setTooltip("Turn auto exposure on or off");
	autoExposure->setToggleState(true, dontSendNotification);
	autoExposure->addListener(this);
	addAndMakeVisible(autoExposure.get());

	// overlaypath button
	overlayPath = std::make_unique<ToggleButton>("Path overlay");
	overlayPath->setBounds(345, 92, 60, 40); //  smaller than this and the JUCE graphics_context goes mental
	overlayPath->setTooltip("Overlay path on video");
	overlayPath->setToggleState(false, dontSendNotification);
	overlayPath->addListener(this);
	addAndMakeVisible(overlayPath.get());

	// two spot tracking button
	twoSpotTracking = std::make_unique<ToggleButton>("Two spot");
	twoSpotTracking->setBounds(400, 72, 60, 40); //  smaller than this and the JUCE graphics_context goes mental
	twoSpotTracking->setTooltip("Use two-spot tracking");
	twoSpotTracking->setToggleState(false, dontSendNotification);
	twoSpotTracking->addListener(this);
	addAndMakeVisible(twoSpotTracking.get());

	// two spot tracking big spot size
	twoSpotBigSpotSize = std::make_unique<TextEditor>("Big spot size");
	twoSpotBigSpotSize->setBounds(403,102, 30, 20); //  smaller than this and the JUCE graphics_context goes mental
	twoSpotBigSpotSize->setTooltip("Max area of the big spot");
	twoSpotBigSpotSize->setFont(font);
	twoSpotBigSpotSize->setText("300", false);
	twoSpotBigSpotSize->addListener(this);
	addAndMakeVisible(twoSpotBigSpotSize.get());

	// two spot tracking small spot size
	twoSpotSmallSpotSize = std::make_unique<TextEditor>("Small spot size");
	twoSpotSmallSpotSize->setBounds(428, 102, 30, 20); //  smaller than this and the JUCE graphics_context goes mental
	twoSpotSmallSpotSize->setTooltip("Max area of the small spot");
	twoSpotSmallSpotSize->setFont(font);
	twoSpotSmallSpotSize->setText("100", false);
	twoSpotSmallSpotSize->addListener(this);
	addAndMakeVisible(twoSpotSmallSpotSize.get());

	// two spot tracking minimum distance
	twoSpotMinDistance = std::make_unique<TextEditor>("2-spot min dist");
	twoSpotMinDistance->setBounds(453, 102, 30, 20); //  smaller than this and the JUCE graphics_context goes mental
	twoSpotMinDistance->setTooltip("Min distance between the 2 LEDs");
	twoSpotMinDistance->setFont(font);
	twoSpotMinDistance->setText("10", false);
	twoSpotMinDistance->addListener(this);
	addAndMakeVisible(twoSpotMinDistance.get());

	// Values used for control ranges & step
	__s32 min, max, step;
	// Brightness slider and label
	brightnessSldr = std::make_unique<CameraControlSlider>(font);
	brightnessSldr->setBounds(210, 30, 50,50);
	brightnessSldr->setActive(false);
	brightnessSldr->addListener(this);
	addAndMakeVisible(brightnessSldr.get());

	brightLbl = std::make_unique<Label>("Brightness", "Brightness");
	brightLbl->setBounds(200, 18, 70, 20);
	brightLbl->setFont(font);
	brightLbl->setEditable (false, false, false);
	brightLbl->setJustificationType(Justification::centred);
	brightLbl->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(brightLbl.get());

	// Contrast slider and label
	contrastSldr = std::make_unique<CameraControlSlider>(font);
	contrastSldr->setBounds(295, 30, 50,50);
	contrastSldr->setActive(false);
	contrastSldr->addListener(this);
	addAndMakeVisible(contrastSldr.get());

	contrLbl = std::make_unique<Label>("Contrast", "Contrast");
	contrLbl->setBounds(285, 18, 70, 20);
	contrLbl->setFont(font);
	contrLbl->setEditable (false, false, false);
	contrLbl->setJustificationType(Justification::centred);
	contrLbl->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(contrLbl.get());

	// Exposure slider and label
	exposureSldr = std::make_unique<CameraControlSlider>(font);
	exposureSldr->setBounds(370, 30, 50,50);
	exposureSldr->setActive(false);
	exposureSldr->addListener(this);
	addAndMakeVisible(exposureSldr.get());

	exposureLbl = std::make_unique<Label>("Exposure", "Exposure");
	exposureLbl->setBounds(360, 18, 70, 20);
	exposureLbl->setFont(font);
	exposureLbl->setEditable(false, false, false);
	exposureLbl->setJustificationType(Justification::centred);
	exposureLbl->setColour(TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(exposureLbl.get());

	// Threshold slider and label
	thresholdSldr = std::make_unique<CameraControlSlider>(font);
	thresholdSldr->setBounds(445, 30, 50, 50);
	thresholdSldr->setActive(false);
	thresholdSldr->addListener(this);
	addAndMakeVisible(thresholdSldr.get());

	thresholdLbl = std::make_unique<Label>("Threshold", "Threshold");
	thresholdLbl->setBounds(435, 18, 70, 20);
	thresholdLbl->setFont(font);
	thresholdLbl->setEditable(false, false, false);
	thresholdLbl->setJustificationType(Justification::centred);
	thresholdLbl->setColour(TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(thresholdLbl.get());

	// Need this to set up the bounds of the sliders for the window box
	std::pair<int,int> resolution = m_proc->getResolution();
	int width = resolution.first;
	int height = resolution.second;

	// LEFT-RIGHT BOUNDING BOX
	leftRightSlider = std::make_unique<FrameControlSlider>(m_proc, font);
	leftRightSlider->setBounds(195, 80, 140, 25);
	leftRightSlider->setRange(0, width, 1);
	leftRightSlider->setMinValue(0);
	leftRightSlider->setMaxValue(width);
	leftRightSlider->setActive(false);
	leftRightSlider->addListener(this);
	addAndMakeVisible(leftRightSlider.get());

	leftRightLbl = std::make_unique<Label>("leftRightLbl", "Left-Right");
	leftRightLbl->setBounds(130, 80, 70, 20);
	leftRightLbl->setFont(font);
	leftRightLbl->setEditable (false, false, false);
	leftRightLbl->setJustificationType(Justification::centred);
	leftRightLbl->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(leftRightLbl.get());

	// TOP-BOTTOM BOUNDING BOX
	topBottomSlider = std::make_unique<FrameControlSlider>(m_proc, font);
	topBottomSlider->setBounds(195, 110, 140, 25);
	topBottomSlider->setRange(0, height, 1);
	topBottomSlider->setMinValue(0);
	topBottomSlider->setMaxValue(height);
	topBottomSlider->setActive(false);
	topBottomSlider->addListener(this);
	addAndMakeVisible(topBottomSlider.get());

	topBottomLbl = std::make_unique<Label>("topBottomLbl", "Top-Bottom");
	topBottomLbl->setBounds(130, 110, 70, 20);
	topBottomLbl->setFont(font);
	topBottomLbl->setEditable (false, false, false);
	topBottomLbl->setJustificationType(Justification::centred);
	topBottomLbl->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(topBottomLbl.get());

	fpslabel = std::make_unique<InfoLabel>(m_proc, font, String("FPS"));
	fpslabel->setBounds(125, 30, 100, 40);
	addAndMakeVisible(fpslabel.get());

	xPoslabel = std::make_unique<InfoLabel>(m_proc, font, String("X"));
	xPoslabel->setBounds(125, 40, 100, 40);
	addAndMakeVisible(xPoslabel.get());

	yPoslabel = std::make_unique<InfoLabel>(m_proc, font, String("Y"));
	yPoslabel->setBounds(125, 50, 100, 40);
	addAndMakeVisible(yPoslabel.get());


}

void PosTrackerEditor::editorShown(Label*, TextEditor& textEditorShown)
{}

void PosTrackerEditor::setInfoValue(InfoLabelType type, double val)
{
	switch (type)
	{
		case InfoLabelType::FPS:
		{
			fpslabel->setInfo(val);
			break;
		}
		case InfoLabelType::XPOS:
		{
			xPoslabel->setInfo(val);
			break;
		}
		case InfoLabelType::YPOS:
		{
			yPoslabel->setInfo(val);
			break;
		}
	}
}

void PosTrackerEditor::sliderValueChanged(Slider * sliderChanged)
{
	if ( sliderChanged == leftRightSlider.get() )
	{
		auto min = sliderChanged->getMinValue();
		m_proc->adjustVideoMask(BORDER::LEFT, min);
		auto max = sliderChanged->getMaxValue();
		m_proc->adjustVideoMask(BORDER::RIGHT, max);
		m_proc->makeVideoMask();
	}
	if ( sliderChanged == topBottomSlider.get() )
	{
		auto min = sliderChanged->getMinValue();
		m_proc->adjustVideoMask(BORDER::TOP, min);
		auto max = sliderChanged->getMaxValue();
		m_proc->adjustVideoMask(BORDER::BOTTOM, max);
		m_proc->makeVideoMask();
	}
	if ( sliderChanged == contrastSldr.get() )
	{
		auto val = sliderChanged->getValue();
		if ( m_proc->isCamReady() )
			m_proc->adjustContrast(val);
	}
	if ( sliderChanged == brightnessSldr.get() )
	{
		auto val = sliderChanged->getValue();
		if ( m_proc->isCamReady() )
			m_proc->adjustBrightness(val);
	}
	if ( sliderChanged == exposureSldr.get() )
	{
		if ( autoExposure->getToggleState() == false )
		{
			auto val = sliderChanged->getValue();
			if ( m_proc->isCamReady() )
				m_proc->adjustExposure(val);
		}
	}
	if ( sliderChanged == thresholdSldr.get() ) {
		auto val = sliderChanged->getValue();
		if ( m_proc->isCamReady() ) {
			m_proc->adjustThreshold(val);
		}
	}
}

void PosTrackerEditor::buttonEvent(Button* button)
{
	if ( button == refreshBtn.get() )
	{
		if ( sourceCombo->getNumItems() > 0 )
		{
			sourceCombo->clear(dontSendNotification);
			resolution->clear(dontSendNotification);
			auto video_devices = m_proc->getDeviceList();
			for (int i = 0; i < video_devices.size(); ++i)
				sourceCombo->addItem(video_devices.at(i), i+1);
		}
	}
	if ( button == showVideo.get() )
	{
		if ( showVideo->getToggleState() == true )
		{
			sourceCombo->setEnabled(false);
			resolution->setEnabled(false);
			refreshBtn->setEnabled(false);
			brightnessSldr->setEnabled(true);
			contrastSldr->setEnabled(true);
			if ( autoExposure->getToggleState() == false )
				exposureSldr->setEnabled(true);
			m_proc->showLiveStream(true);
			m_proc->openCamera();
			m_proc->startStreaming();

		}
		else if ( showVideo->getToggleState() == false )
		{
			sourceCombo->setEnabled(true);
			resolution->setEnabled(true);
			refreshBtn->setEnabled(true);
			brightnessSldr->setEnabled(false);
			contrastSldr->setEnabled(false);
			if ( autoExposure->getToggleState() == false )
				exposureSldr->setEnabled(false);
			m_proc->stopStreaming();
			m_proc->showLiveStream(false);
		}
	}
	if ( button == autoExposure.get() )
	{
		if ( autoExposure->getToggleState() == true )
		{
			m_proc->changeExposureTo(V4L2_EXPOSURE_AUTO);
			m_proc->autoExposure(true);
		}
		else if ( autoExposure->getToggleState() == false ) {
			m_proc->autoExposure(false);
			m_proc->changeExposureTo(V4L2_EXPOSURE_MANUAL);
		}
	}

	if ( button == overlayPath.get() )
	{
		if ( overlayPath->getToggleState() == true )
			m_proc->overlayPath(true);
		else
			m_proc->overlayPath(false);
	}
	if ( button == twoSpotTracking.get() ) {
		if ( twoSpotTracking->getToggleState() == true )
			m_proc->doTwoSpotTracking(true);
		else
			m_proc->doTwoSpotTracking(false);
	}
}

void PosTrackerEditor::comboBoxChanged(ComboBox* cb)
{

	if ( cb == sourceCombo.get() )
	{
		if ( resolution->getNumItems() > 0 )
			resolution->clear();
		int idx = cb->getSelectedId();
		std::string dev_name = cb->getItemText(idx-1).toStdString();
		if (m_proc->getDeviceName() != dev_name) {
			m_proc->createNewCamera(dev_name);
			m_proc->getDeviceName();
		}
		auto fmts = m_proc->getDeviceFormats();
		for (int i = 0; i < fmts.size(); ++i)
			resolution->addItem(fmts[i], i+1);
	}
	else if ( cb == resolution.get() )
	{
		int idx = cb->getSelectedId();
		auto formatId = cb->getItemText(idx-1).toStdString();
		m_proc->setDeviceFormat(formatId);
		showVideo->setEnabled(true);
		updateSettings();
	}
}

void PosTrackerEditor::textEditorReturnKeyPressed(TextEditor & te) {
	if ( &te == twoSpotMinDistance.get() ) {
		auto mindist = getTextAsUnsignedInt(te);
		if (mindist == 0)
			m_proc->twoSpotMinDistance(50);
		else
			m_proc->twoSpotMinDistance(mindist);
	}
	if ( &te == twoSpotSmallSpotSize.get() ) {
		auto spotsize = getTextAsUnsignedInt(te);
		if (spotsize == 0)
			m_proc->twoSpotSmallSpotSize(300);
		else
			m_proc->twoSpotSmallSpotSize(spotsize);

	}
	if ( &te == twoSpotBigSpotSize.get() ) {
		auto spotsize = getTextAsUnsignedInt(te);
		if (spotsize == 0)
			m_proc->twoSpotBigSpotSize(100);
		else
			m_proc->twoSpotBigSpotSize(spotsize);
	}
}

unsigned int PosTrackerEditor::getTextAsUnsignedInt(TextEditor & te) {
	std::string str = te.getText().toStdString();
	if ( str.empty() )
		return 0;
	return std::stoul(str);
}

void PosTrackerEditor::updateSettings()
{
	std::string dev_name = m_proc->getDeviceName();
	if ( ! dev_name.empty() )
	{
		int nDevices = sourceCombo->getNumItems();
		for (int i = 0; i < nDevices; ++i)
		{
			std::string name = sourceCombo->getItemText(i).toStdString();
			if ( dev_name.compare(name) == 0 )
				sourceCombo->setSelectedId(i+1, sendNotificationSync); // i.e. dont trigger comboBoxChanged
		}
		std::string thisfmt = m_proc->getFormatName();
		int nFormats = resolution->getNumItems();
		for (int i = 0; i < nFormats; ++i)
		{
			std::string name = resolution->getItemText(i).toStdString();
			if ( thisfmt.compare(name) == 0 )
			{
				resolution->setSelectedId(i+1, dontSendNotification);
				resolution->setEnabled(true);
			}
		}
	}
	else
	{
		return;
	}
	if ( m_proc->isCamReady() )
	{
		// THIS WHOLE CHUNK NEEDS IMPROVING
		// mainly deal with getting the sliders values correctly
		std::pair<int,int> resolution = m_proc->getResolution();
		int width = resolution.first;
		int height = resolution.second;

		int left = m_proc->getVideoMask(BORDER::LEFT);
		int right = m_proc->getVideoMask(BORDER::RIGHT);
		int top = m_proc->getVideoMask(BORDER::TOP);
		int bottom = m_proc->getVideoMask(BORDER::BOTTOM);


		leftRightSlider->setActive(true);
		leftRightSlider->setRange(0, width, 1);
		// int width_frac = width * 0.1;
		leftRightSlider->setMinValue(left);
		leftRightSlider->setMaxValue(right);

		topBottomSlider->setActive(true);
		topBottomSlider->setRange(0, height, 1);
		// int height_frac = height * 0.1;
		topBottomSlider->setMinValue(top);
		topBottomSlider->setMaxValue(bottom);

		thresholdSldr->setActive(true);
		Array<double>thresh_range{double(0), double(255)};
		thresholdSldr->setValues(thresh_range);
		thresholdSldr->setRange(0, 255, 1);
		int new_val = m_proc->getThreshold();
		m_proc->adjustThreshold(new_val);

		m_proc->makeVideoMask();

		// Ranges and step for controls
		__s32 min = 0;
		__s32 max = 100;
		__s32 step = 10;
	    // return code for control (0 ok, 1 fucked)
	    int control_ok = 1;
		// ------------- BRIGHTNESS ------------------
	    control_ok = m_proc->getControlValues(V4L2_CID_BRIGHTNESS, min, max, step);
	    Array<double>brightness_range{double(min), double(max)};
	    if ( control_ok == 0 ) { // all good
	    	int new_val = m_proc->getBrightness();
		    brightnessSldr->setValue(new_val);
		    brightnessSldr->setValues(brightness_range);
		    brightnessSldr->setRange(min, max, step);
		    m_proc->adjustBrightness(new_val);
		    brightnessSldr->setActive(true);
	    }
	    else
		    brightnessSldr->setActive(false);
	    // CONTRAST
	    control_ok = m_proc->getControlValues(V4L2_CID_CONTRAST, min, max, step);
	    Array<double>contrast_range{double(min), double(max)};
	    if ( control_ok == 0 ) {
	    	int new_val = m_proc->getContrast();
		    contrastSldr->setValue(new_val);
		    contrastSldr->setValues(contrast_range);
		    contrastSldr->setRange(min, max, step);
		    m_proc->adjustContrast(new_val);
		    contrastSldr->setActive(true);
	    }
	    else
	    	contrastSldr->setActive(false);

	    // EXPOSURE
	    control_ok = m_proc->getControlValues(V4L2_CID_EXPOSURE_ABSOLUTE, min, max, step);
	    Array<double>exposure_range{double(min), double(max)};
	    bool use_auto_exposure = m_proc->autoExposure();
	    if ( (control_ok == 0) && ( ! use_auto_exposure ) ) {
	    	int new_val = m_proc->getExposure();
	    	autoExposure->setToggleState(false, sendNotification);
		    exposureSldr->setValue(new_val);
		    exposureSldr->setValues(exposure_range);
		    exposureSldr->setRange(min, max, step);
		    m_proc->adjustExposure(new_val);
		    exposureSldr->setActive(true);
	    }
	    else {
	    	exposureSldr->setActive(false);
	    	autoExposure->setToggleState(true, sendNotification);
	    }

	    // overlay path?
	    bool path_overlay = m_proc->overlayPath();
	    if ( path_overlay )
	    	overlayPath->setToggleState(true, sendNotification);
	    else
	    	overlayPath->setToggleState(false, sendNotification);

		// two spot tracking?
	    bool two_spot = m_proc->doTwoSpotTracking();
	    if ( two_spot )
	    	twoSpotTracking->setToggleState(true, sendNotification);
	    else
	    	twoSpotTracking->setToggleState(false, sendNotification);

	    showVideo->setEnabled(true);
	}
}

void PosTrackerEditor::update()
{}

/*
------------------ CAMERACONTROLSLIDER CLASS -------------------
			Control brightness, contrast and exposure
*/

CameraControlSlider::CameraControlSlider(Font f) : Slider("name"), font(f)
{
	setSliderStyle(Slider::Rotary);
	setRange(0, 100, 1);
	setValue(50);
	setTextBoxStyle(Slider::NoTextBox, false, 20, 20);
	Array<double> v{0,100};
	setValues(v);
	setVelocityBasedMode(true);

	upButton = std::make_unique<TriangleButton>(1);
	upButton->addListener(this);
	upButton->setBounds(16, 21, 10, 8);
	addAndMakeVisible(upButton.get());

	downButton = std::make_unique<TriangleButton>(2);
	downButton->addListener(this);
	downButton->setBounds(24, 21, 10, 8);
	addAndMakeVisible(downButton.get());
}

void CameraControlSlider::buttonClicked(Button * btn) {
	if ( btn == upButton.get() ) {
		auto val = getValue();
		auto inc = getInterval();
		setValue(val + inc);
	}
	if ( btn == downButton.get() ) {
		auto val = getValue();
		auto inc = getInterval();
		setValue(val - inc);
	}
}
void CameraControlSlider::setActive(bool active)
{
	m_isActive = active;
	repaint();
}

void CameraControlSlider::setValues(Array<double> vals)
{
	valueArray = vals;
}

void CameraControlSlider::paint(Graphics & g)
{
	ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,
		Colour(80, 80, 80), 0.0, 40.0f, false);

	Path p;
	p.addPieSegment(3, 3, getWidth() - 6, getHeight() - 6, -(3 * double_Pi)/4, (3 * double_Pi)/4, 0.5);

	g.setGradientFill(grad);
	g.fillPath(p);

	String valueString;

	if ( m_isActive )
	{
		p = makeRotaryPath(getValue());
		g.setColour(Colour(240, 179, 12));
		g.fillPath(p);

		valueString = String((int)getValue());
	}
	else
	{

		valueString = "";

		for (int i = 0; i < valueArray.size(); i++)
		{
			p = makeRotaryPath(valueArray[i]);
			g.setColour(Colours::lightgrey.withAlpha(0.4f));
			g.fillPath(p);
			valueString = String((int)valueArray.getLast());
		}

	}

	font.setHeight(9.0);
	g.setFont(font);
	int stringWidth = font.getStringWidth(valueString);

	g.setFont(font);

	g.setColour(Colours::darkgrey);
	g.drawSingleLineText(valueString, getWidth() / 2 - stringWidth / 2, getHeight() - 9);
}

Path CameraControlSlider::makeRotaryPath(double val)
{
	Path p;

	double start;
	double range;
	start = -(3 * double_Pi)/4;
	range = start + (3 * double_Pi)/2 * (double(std::abs(val)) / std::abs(valueArray[1]));
	p.addPieSegment(6, 6, getWidth() - 12, getHeight() - 12, start, range, 0.65);

	return p;
}

/*
------------------ FRAMECONTROLSLIDER CLASS -------------------
			Control top/ bottom, left/ right boundaries
*/


FrameControlSlider::FrameControlSlider(PosTracker * proc, Font f) : Slider("name"), font(f), m_proc(proc)
{
	setSliderStyle(Slider::TwoValueHorizontal);
	setRange(0, 100, 1);
	setMinValue(0);
	setMaxValue(100);
	setTextBoxStyle(Slider::NoTextBox, false, 40, 20);

	auto l = std::make_unique<Label>("min");
	l->setBounds(0, getHeight()+12, 40, 10);
	font.setHeight(9.0);
	l->setFont(f);
	l->setEditable(true);
	l->addListener(this);
	l->setText(String("0000"), dontSendNotification);
	addAndMakeVisible(l.get());
	borderlbl.push_back(std::move(l));

	l = std::make_unique<Label>("max");
	l->setBounds(110, getHeight()+12, 40, 10);
	l->setFont(f);
	l->setEditable(true);
	l->addListener(this);
	l->setText(String("9999"), dontSendNotification);
	addAndMakeVisible(l.get());
	borderlbl.push_back(std::move(l));

	setValue(0, 0);
	setValue(1, 1);
}

void FrameControlSlider::setActive(bool active)
{
	m_isActive = active;
	repaint();
}

void FrameControlSlider::paint(Graphics & g)
{
	ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,
		Colour(80, 80, 80), 0.0, 40.0f, false);

	Path p;
	p.addRoundedRectangle(12, 7, getWidth() - 20, getHeight() / 5, 2);

	g.setGradientFill(grad);
	g.fillPath(p);

	int minVal;
	int maxVal;

	if ( m_isActive )
	{
		p = makeFilledPath(getMinValue(), 0);
		g.setColour(Colour(240, 179, 12));
		g.fillPath(p);

		p = makeArrowHead(getMinValue(), 0);
		g.setColour(Colours::darkgrey);
		g.fillPath(p);

		p = makeFilledPath(getMaxValue(), 1);
		g.setColour(Colour(240, 179, 12));
		g.fillPath(p);

		p = makeArrowHead(getMaxValue(), 1);
		g.setColour(Colours::darkgrey);
		g.fillPath(p);

		minVal = (int)getMinValue();
		maxVal = (int)getMaxValue();
	}
	else
	{
		minVal = 0;
		maxVal = 0;
	}

	font.setHeight(9.0);
	g.setFont(font);
	g.setColour(Colours::darkgrey);

	setValue(0, minVal);
	setValue(1, maxVal);
}

Path FrameControlSlider::makeArrowHead(double value, int type)
{
	Path p;
	if ( type == 0 ) // min
	{
		double v = (value / getMaximum()) * (getWidth()-20);
		p.addTriangle(v+12, 6, v+16, 0, v+8, 0);
	}
	else if ( type == 1 ) // max
	{
		double v = 12 + (value / getMaximum()) * (getWidth()-20);
		p.addTriangle(v, 6, v+4, 0, v-4, 0);
	}

	return p;
}

Path FrameControlSlider::makeFilledPath(double value, int type)
{
	Path p;

	if ( type == 0 ) // min
	{
		p.addRoundedRectangle(12, 7, (value / getMaximum()) * (getWidth() - 20), getHeight() / 5, 2);
	}
	else if ( type == 1) // max
	{
		double x = 12 + (value / getMaximum()) * (getWidth() - 20);
		double width = 12 + (getWidth() - 20) - x;

		p.addRoundedRectangle(x, 7, width, getHeight() / 5, 2);
	}
	return p;
}

void FrameControlSlider::labelTextChanged(Label * label)
{

}

void FrameControlSlider::handleAsyncUpdate()
{
	borderlbl[0]->setText(labelText[0], dontSendNotification);
}

void FrameControlSlider::setValue(int idx, int val)
{
	if ( m_proc->isStreaming() )
	{
		triggerAsyncUpdate();
	}
	else
	{
		borderlbl[idx]->setText(String(val), dontSendNotification);
	}
}


/*
------------------ INFOLABEL CLASS -------------------
			Displays changes in fps, x and y pos
*/



InfoLabel::InfoLabel(PosTracker * proc, Font f, String name)
	: m_proc(proc), m_font(f)
{
	lbl = std::make_unique<Label>(name, name);
	lbl->setBounds(0, 5, 30, 20);
	lbl->setEditable(false);
	lbl->setFont(m_font);
	lbl->setColour(TextEditor::textColourId, Colours::grey);
	lbl->setJustificationType(Justification::right);
	addAndMakeVisible(lbl.get());

	lblvalue = std::make_unique<Label>("value");
	lblvalue->setBounds(30, 5, 50, 20);
	lblvalue->setEditable(false);
	lblvalue->setFont(m_font);
	setInfo(0.0);
	addAndMakeVisible(lblvalue.get());
}

void InfoLabel::handleAsyncUpdate()
{
	lblvalue->setText(String(infoVal,1), dontSendNotification);
}
void InfoLabel::setEnable(bool val)
{
	lblvalue->setEnabled(val);
}
void InfoLabel::setInfo(double val)
{
	if ( m_proc->isStreaming() )
	{
		infoVal = val;
		triggerAsyncUpdate();
	}
	else
		lblvalue->setText(String(val, 1), dontSendNotification);
}