#ifndef POSTRACKEREDITOR_H_
#define POSTRACKEREDITOR_H_

#include <memory>
#include <EditorHeaders.h>
#include "PosTracker.h"

#include "../common.h"

class CameraControlSlider;
class FrameControlSlider;
class InfoLabel;

enum class InfoLabelType
{
	FPS,
	XPOS,
	YPOS
};

class PosTrackerEditor : public GenericEditor, public ComboBox::Listener, public TextEditor::Listener
{
public:
	PosTrackerEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors);
	~PosTrackerEditor() {};

	void editorShown(Label*, TextEditor& textEditorShown);

	void comboBoxChanged(ComboBox* cb);
	void buttonEvent(Button* button);
	void textEditorReturnKeyPressed(TextEditor &);
	unsigned int getTextAsUnsignedInt(TextEditor &);
	void updateSettings();
	void update();

	void setInfoValue(InfoLabelType, double);

	void sliderValueChanged(Slider * sliderChanged);

private:
	PosTracker * m_proc;

	std::unique_ptr<ComboBox> sourceCombo;
	std::unique_ptr<ComboBox> resolution;
	std::unique_ptr<UtilityButton> refreshBtn;

	std::unique_ptr<ToggleButton> showVideo;
	std::unique_ptr<ToggleButton> autoExposure;
	std::unique_ptr<ToggleButton> overlayPath;
	std::unique_ptr<ToggleButton> twoSpotTracking;

	std::unique_ptr<TextEditor> twoSpotMinDistance;
	std::unique_ptr<TextEditor> twoSpotBigSpotSize;
	std::unique_ptr<TextEditor> twoSpotSmallSpotSize;

	std::unique_ptr<CameraControlSlider> brightnessSldr;
	std::unique_ptr<CameraControlSlider> contrastSldr;
	std::unique_ptr<CameraControlSlider> exposureSldr;
	std::unique_ptr<CameraControlSlider> thresholdSldr;

	std::unique_ptr<Label> brightnessVal;
	std::unique_ptr<Label> contrastVal;
	std::unique_ptr<Label> exposureVal;
	std::unique_ptr<Label> thresholdLbl;

	std::unique_ptr<Label> brightLbl;
	std::unique_ptr<Label> contrLbl;
	std::unique_ptr<Label> exposureLbl;

	std::unique_ptr<FrameControlSlider> leftRightSlider;
	std::unique_ptr<FrameControlSlider> topBottomSlider;

	std::unique_ptr<Label> leftRightLbl;
	std::unique_ptr<Label> topBottomLbl;

	std::unique_ptr<InfoLabel> fpslabel;
	std::unique_ptr<InfoLabel> xPoslabel;
	std::unique_ptr<InfoLabel> yPoslabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PosTrackerEditor);
};

/*
Define some classes to make things look nice and play nicer with the 
webcam

*/

class CameraControlSlider : public Slider, public Button::Listener
{
public:
	CameraControlSlider(Font f);
	~CameraControlSlider() {};

	void setActive(bool);
	bool isActive() { return m_isActive; }
	void setValues(Array<double>);
	void buttonClicked(Button * btn);

private:
	void paint (Graphics & g) override;
	Path makeRotaryPath(double);
	Font font;
	bool m_isActive;
	Array<double> valueArray;
	std::unique_ptr<TriangleButton> upButton;
	std::unique_ptr<TriangleButton> downButton;
};

class FrameControlSlider : public Slider, public Label::Listener, public AsyncUpdater
{
public:
	FrameControlSlider(PosTracker * proc, Font f);
	~FrameControlSlider() {};

	void setActive(bool);
	bool isActive() { return m_isActive; }
	void labelTextChanged(Label *) override;
	void setValue(int idx, int val);
	void handleAsyncUpdate() override;

private:
	void paint(Graphics & g);
	Path makeFilledPath(double, int);
	Path makeArrowHead(double, int);
	std::vector<std::unique_ptr<Label>> borderlbl;
	String labelText[2];

	PosTracker * m_proc;
	Font font;
	bool m_isActive;
};

class InfoLabel : public Component, public Label::Listener, public AsyncUpdater
{
public:
	InfoLabel(PosTracker * proc, Font f, String name);
	~InfoLabel() {};
	 void paint(Graphics &) override {};
	 void labelTextChanged(Label *) override {};
	 void handleAsyncUpdate();
	 void setEnable(bool);
	 void setInfo(double);
private:
	std::unique_ptr<Label> lbl;
	std::unique_ptr<Label> lblvalue;
	double infoVal = 0.0;

	PosTracker * m_proc;
	Font m_font;
};

#endif