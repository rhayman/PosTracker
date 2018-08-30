#include "PosTrackerEditor.h"
#include "PosTracker.h"


PosTrackerEditor::PosTrackerEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors=true)
{
	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
	MemoryInputStream mis(silk, silksize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	Font font = Font(typeface);

	desiredWidth = 425;
	m_proc = (PosTracker*)parentNode;

	// Video source stuff
	addAndMakeVisible(sourceCombo = new ComboBox("sourceCombo"));
	sourceCombo->setBounds(5, 30, 120, 20); // x,y,w,h
	sourceCombo->setTooltip("The video device (e.g. /dev/video0)");
	sourceCombo->setEditableText(false);
	sourceCombo->setJustificationType(Justification::centredLeft);
	sourceCombo->setTextWhenNothingSelected("Device");
	sourceCombo->setTextWhenNoChoicesAvailable("Device");

	auto video_devices = m_proc->getDeviceList();
	for (int i = 0; i < video_devices.size(); ++i)
		sourceCombo->addItem(video_devices.at(i), i+1);

	sourceCombo->addListener(this);

	addAndMakeVisible(resolution = new ComboBox ("resolutionMenu"));
	resolution->setBounds(5, 55, 120, 20);
	resolution->setTooltip (("The size and frame rate of the video output (e.g. 640 x 480, 30fps)"));
	resolution->setEditableText (false);
	resolution->setJustificationType (Justification::centredLeft);
	resolution->setTextWhenNothingSelected (("Resolution/ fps"));
	resolution->setTextWhenNoChoicesAvailable (("Resolution / fps"));
	resolution->addListener(this);

	addAndMakeVisible(refreshBtn = new UtilityButton("Refresh", Font ("Small Text", 10, Font::plain)));
	refreshBtn->setBounds(5, 80, 60, 20);
	refreshBtn->setTooltip("Refresh the list of devices");
	refreshBtn->addListener(this);

	addAndMakeVisible(showVideo = new ToggleButton("Show video"));
	showVideo->setBounds(5, 105, 100, 20);
	showVideo->setTooltip("Show the video stream");
	showVideo->addListener(this);
	showVideo->setEnabled(false);

	font.setHeight(8);

	// autoexposure button
	addAndMakeVisible(autoExposure = new ToggleButton("Auto exposure"));
	autoExposure->setBounds(345, 72, 60, 40); //  smaller than this and the JUCE graphics_context goes mental
	autoExposure->setTooltip("Turn auto exposure on or off");
	autoExposure->setToggleState(true, dontSendNotification);
	autoExposure->addListener(this);

	// overlaypath button
	addAndMakeVisible(overlayPath = new ToggleButton("Path overlay"));
	overlayPath->setBounds(345, 92, 60, 40); //  smaller than this and the JUCE graphics_context goes mental
	overlayPath->setTooltip("Overlay path on video");
	overlayPath->setToggleState(false, dontSendNotification);
	overlayPath->addListener(this);

	// Values used for control ranges & step
	__s32 min, max, step;
	// Brightness slider and label
	addAndMakeVisible(brightnessSldr = new CameraControlSlider(font));
	brightnessSldr->setBounds(210, 30, 50,50);
	brightnessSldr->setActive(false);
	brightnessSldr->addListener(this);

	addAndMakeVisible(brightLbl = new Label("Brightness", "Brightness"));
	brightLbl->setBounds(200, 18, 70, 20);
	brightLbl->setFont(font);
	brightLbl->setEditable (false, false, false);
	brightLbl->setJustificationType(Justification::centred);
	brightLbl->setColour (TextEditor::textColourId, Colours::grey);

	// Contrast slider and label
	addAndMakeVisible(contrastSldr = new CameraControlSlider(font));
	contrastSldr->setBounds(295, 30, 50,50);
	contrastSldr->setActive(false);
	contrastSldr->addListener(this);

	addAndMakeVisible(contrLbl = new Label("Contrast", "Contrast"));
	contrLbl->setBounds(285, 18, 70, 20);
	contrLbl->setFont(font);
	contrLbl->setEditable (false, false, false);
	contrLbl->setJustificationType(Justification::centred);
	contrLbl->setColour (TextEditor::textColourId, Colours::grey);

	// Exposure slider and label
	addAndMakeVisible(exposureSldr = new CameraControlSlider(font));
	exposureSldr->setBounds(370, 30, 50,50);
	exposureSldr->setActive(false);
	exposureSldr->addListener(this);

	addAndMakeVisible(exposureLbl = new Label("Exposure", "Exposure"));
	exposureLbl->setBounds(360, 18, 70, 20);
	exposureLbl->setFont(font);
	exposureLbl->setEditable(false, false, false);
	exposureLbl->setJustificationType(Justification::centred);
	exposureLbl->setColour(TextEditor::textColourId, Colours::grey);

	std::pair<int,int> resolution = m_proc->getResolution();
	int width = resolution.first;
	int height = resolution.second;

	// LEFT-RIGHT BOUNDING BOX
	addAndMakeVisible(leftRightSlider = new FrameControlSlider(m_proc, font));
	leftRightSlider->setBounds(195, 80, 140, 25);
	leftRightSlider->setRange(0, width, 1);
	leftRightSlider->setMinValue(0);
	leftRightSlider->setMaxValue(width);
	leftRightSlider->setActive(false);
	leftRightSlider->addListener(this);

	addAndMakeVisible(leftRightLbl = new Label("leftRightLbl", "Left-Right"));
	leftRightLbl->setBounds(130, 80, 70, 20);
	leftRightLbl->setFont(font);
	leftRightLbl->setEditable (false, false, false);
	leftRightLbl->setJustificationType(Justification::centred);
	leftRightLbl->setColour (TextEditor::textColourId, Colours::grey);

	// TOP-BOTTOM BOUNDING BOX
	addAndMakeVisible(topBottomSlider = new FrameControlSlider(m_proc, font));
	topBottomSlider->setBounds(195, 110, 140, 25);
	topBottomSlider->setRange(0, height, 1);
	topBottomSlider->setMinValue(0);
	topBottomSlider->setMaxValue(height);
	topBottomSlider->setActive(false);
	topBottomSlider->addListener(this);

	addAndMakeVisible(topBottomLbl = new Label("topBottomLbl", "Top-Bottom"));
	topBottomLbl->setBounds(130, 110, 70, 20);
	topBottomLbl->setFont(font);
	topBottomLbl->setEditable (false, false, false);
	topBottomLbl->setJustificationType(Justification::centred);
	topBottomLbl->setColour (TextEditor::textColourId, Colours::grey);

	addAndMakeVisible(fpslabel = new InfoLabel(m_proc, font, String("FPS")));
	fpslabel->setBounds(125, 30, 100, 40);

	addAndMakeVisible(xPoslabel = new InfoLabel(m_proc, font, String("X")));
	xPoslabel->setBounds(125, 40, 100, 40);

	addAndMakeVisible(yPoslabel = new InfoLabel(m_proc, font, String("Y")));
	yPoslabel->setBounds(125, 50, 100, 40);


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
	if ( sliderChanged == leftRightSlider )
	{
		auto min = sliderChanged->getMinValue();
		m_proc->adjustVideoMask(BORDER::LEFT, min);
		auto max = sliderChanged->getMaxValue();
		m_proc->adjustVideoMask(BORDER::RIGHT, max);
		m_proc->makeVideoMask();
	}
	if ( sliderChanged == topBottomSlider )
	{
		auto min = sliderChanged->getMinValue();
		m_proc->adjustVideoMask(BORDER::TOP, min);
		auto max = sliderChanged->getMaxValue();
		m_proc->adjustVideoMask(BORDER::BOTTOM, max);
		m_proc->makeVideoMask();
	}
	if ( sliderChanged == contrastSldr )
	{
		auto val = sliderChanged->getValue();
		if ( m_proc->isCamReady() )
			m_proc->adjustBrightness(val);
	}
	if ( sliderChanged == brightnessSldr )
	{
		auto val = sliderChanged->getValue();
		if ( m_proc->isCamReady() )
			m_proc->adjustContrast(val);
	}
	if ( sliderChanged == exposureSldr )
	{
		if ( autoExposure->getToggleState() == false )
		{
			auto val = sliderChanged->getValue();
			if ( m_proc->isCamReady() )
				m_proc->adjustExposure(val);
		}
	}
}

void PosTrackerEditor::buttonEvent(Button* button)
{
	if (button == refreshBtn)
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
	if ( button == showVideo )
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
	if ( button == autoExposure )
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

	if ( button == overlayPath )
	{
		if ( overlayPath->getToggleState() == true )
			m_proc->overlayPath(true);
		else
			m_proc->overlayPath(false);
	}
}

void PosTrackerEditor::comboBoxChanged(ComboBox* cb)
{

	if (cb == sourceCombo)
	{
		if ( resolution->getNumItems() > 0 )
			resolution->clear();
		int idx = cb->getSelectedId();
		std::string dev_name = cb->getItemText(idx-1).toStdString();
		if ( m_proc->getDeviceName() != dev_name )
			m_proc->createNewCamera(dev_name);
		auto fmts = m_proc->getDeviceFormats();
		for (int i = 0; i < fmts.size(); ++i)
			resolution->addItem(fmts[i], i+1);
	}
	else if (cb == resolution)
	{
		int idx = cb->getSelectedId();
		auto formatId = cb->getItemText(idx-1).toStdString();
		m_proc->setDeviceFormat(formatId);
		showVideo->setEnabled(true);
		updateSettings();
	}
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
		// DEAL WITH THIS PROPERLY
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

		m_proc->makeVideoMask();

		// Ranges and step for controls
	    __s32 min, max, step;
	    // return code for control (0 ok, 1 fucked)
	    int control_ok;
		// ------------- BRIGHTNESS ------------------
	    control_ok = m_proc->getControlValues(V4L2_CID_BRIGHTNESS, min, max, step);
	    Array<double>v{double(min), double(max)};
	    if ( control_ok == 0 ) { // all good
	    	int new_val = m_proc->getBrightness();
		    brightnessSldr->setValue(new_val);
		    brightnessSldr->setValues(v);
		    brightnessSldr->setRange(min, max, step);
		    m_proc->adjustBrightness(new_val);
		    brightnessSldr->setActive(true);
	    }
	    else
		    brightnessSldr->setActive(false);
	    // CONTRAST
	    control_ok = m_proc->getControlValues(V4L2_CID_CONTRAST, min, max, step);
	    if ( control_ok == 0 ) {
	    	int new_val = m_proc->getContrast();
		    contrastSldr->setValue(new_val);
		    v = {double(min), double(max)};
		    contrastSldr->setValues(v);
		    contrastSldr->setRange(min, max, step);
		    m_proc->adjustContrast(new_val);
		    contrastSldr->setActive(true);
	    }
	    else
	    	contrastSldr->setActive(false);

	    // EXPOSURE
	    control_ok = m_proc->getControlValues(V4L2_CID_EXPOSURE_ABSOLUTE, min, max, step);
	    bool use_auto_exposure = m_proc->autoExposure();
	    if ( (control_ok == 0) && ( ! use_auto_exposure ) ) {
	    	int new_val = m_proc->getExposure();
	    	autoExposure->setToggleState(false, sendNotification);
		    exposureSldr->setValue(new_val);
		    v = {double(min), double(max)};
		    exposureSldr->setValues(v);
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

	addAndMakeVisible(upButton = new TriangleButton(1));
	upButton->addListener(this);
	upButton->setBounds(16, 21, 10, 8);

	addAndMakeVisible(downButton = new TriangleButton(2));
	downButton->addListener(this);
	downButton->setBounds(24, 21, 10, 8);
}

void CameraControlSlider::buttonClicked(Button * btn) {
	if ( btn == upButton ) {
		auto val = getValue();
		auto inc = getInterval();
		setValue(val + inc);
	}
	if ( btn == downButton ) {
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

	Label * l = new Label("min");
	l->setBounds(0, getHeight()+12, 40, 10);
	font.setHeight(9.0);
	l->setFont(f);
	l->setEditable(true);
	l->addListener(this);
	l->setText(String("0000"), dontSendNotification);
	addAndMakeVisible(l);
	borderlbl[0] = l;

	l = new Label("max");
	l->setBounds(110, getHeight()+12, 40, 10);
	l->setFont(f);
	l->setEditable(true);
	l->addListener(this);
	l->setText(String("9999"), dontSendNotification);
	addAndMakeVisible(l);
	borderlbl[1] = l;

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
	addAndMakeVisible(lbl = new Label(name, name));
	lbl->setBounds(0, 5, 30, 20);
	lbl->setEditable(false);
	lbl->setFont(m_font);
	lbl->setColour(TextEditor::textColourId, Colours::grey);
	lbl->setJustificationType(Justification::right);

	addAndMakeVisible(lblvalue = new Label("value"));
	lblvalue->setBounds(30, 5, 50, 20);
	lblvalue->setEditable(false);
	lblvalue->setFont(m_font);
	setInfo(0.0);
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