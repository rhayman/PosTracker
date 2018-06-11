#ifndef POSTRACKEREDITOR_H_
#define POSTRACKEREDITOR_H_

#include <EditorHeaders.h>

#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>
#include "PosTrackerCanvas.h"
#include "common.h"

class CameraControlSlider;
class FrameControlSlider;
class InfoLabel;
class PosTrackerCanvas;

enum class InfoLabelType
{
	FPS,
	XPOS,
	YPOS
};

class PosTrackerEditor : public VisualizerEditor, public ComboBox::Listener
{
public:
	PosTrackerEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors);
	~PosTrackerEditor() {};

	void editorShown(Label*, TextEditor& textEditorShown);

	void comboBoxChanged(ComboBox* cb);
	void buttonEvent(Button* button) override;
	void updateSettings() override;
	void update();

	Visualizer * createNewCanvas();

	TrackerType getTrackerType();

	void setInfoValue(InfoLabelType, double);

	void sliderValueChanged(Slider * sliderChanged) override;

private:
	PosTracker * m_proc;
	PosTrackerCanvas * trackerCanvas;

	ScopedPointer<ComboBox> sourceCombo;
	ScopedPointer<ComboBox> resolution;
	ScopedPointer<UtilityButton> refreshBtn;
	ScopedPointer<ComboBox> trackerTypeCombo;

	ScopedPointer<ToggleButton> showVideo;
	ScopedPointer<ToggleButton> autoExposure;
	ScopedPointer<ToggleButton> overlayPath;

	ScopedPointer<CameraControlSlider> brightnessSldr;
	ScopedPointer<CameraControlSlider> contrastSldr;
	ScopedPointer<CameraControlSlider> exposureSldr;

	ScopedPointer<Label> brightnessVal;
	ScopedPointer<Label> contrastVal;
	ScopedPointer<Label> exposureVal;

	ScopedPointer<Label> brightLbl;
	ScopedPointer<Label> contrLbl;
	ScopedPointer<Label> exposureLbl;

	ScopedPointer<FrameControlSlider> leftRightSlider;
	ScopedPointer<FrameControlSlider> topBottomSlider;

	ScopedPointer<Label> leftRightLbl;
	ScopedPointer<Label> topBottomLbl;

	ScopedPointer<InfoLabel> fpslabel;
	ScopedPointer<InfoLabel> xPoslabel;
	ScopedPointer<InfoLabel> yPoslabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PosTrackerEditor);
};

/*
Define some classes to make things look nice and play nicer with the 
webcam

*/

class CameraControlSlider : public Slider
{
public:
	CameraControlSlider(Font f);
	~CameraControlSlider() {};

	void setActive(bool);
	bool isActive() { return m_isActive; }
	void setValues(Array<double>);

private:
	void paint (Graphics & g) override;
	Path makeRotaryPath(double);
	Font font;
	bool m_isActive;
	Array<double> valueArray;
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
	ScopedPointer<Label> borderlbl[2];
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
	ScopedPointer<Label> lbl;
	ScopedPointer<Label> lblvalue;
	double infoVal = 0.0;

	PosTracker * m_proc;
	Font m_font;
};

#endif