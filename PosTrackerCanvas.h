#ifndef POSTRACKERCANVAS_H_
#define POSTRACKERCANVAS_H_

#include "PosTracker.h"
#include "PosTrackerEditor.h"
#include <EditorHeaders.h>
#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>

class PosTracker;
class PosTrackerEditor;
class BoostingComponent;

class PosTrackerCanvas : public Visualizer, public Button::Listener, public Label::Listener, public ComboBox::Listener, public Slider::Listener
{
public:
	PosTrackerCanvas(PosTracker * parentNode);

	void paint(Graphics & g);
	void refresh();
	void beginAnimation();
	void endAnimation();
	void refreshState();
	void resized();

	void setParameter(int, float) {};
	void setParameter(int, int, int, float) {};

	void update();

	void labelTextChanged(Label* labelChanged);

	void comboBoxChanged(ComboBox * comboChanged);
	void sliderValueChanged(Slider * sliderChanged);
	void buttonClicked(Button * button);

	void makeBoostingPanel();
	void makeKCFPanel();

	ScopedPointer<Slider> addSliderToGroup(ScopedPointer<GroupComponent> group, double min, double max, double step);
	void addLabelToGroup();

private:
	PosTracker * processor;
	PosTrackerEditor * editor;

	ScopedPointer<Viewport> displayView;

	ScopedPointer<ComboBox> trackerType;

	ScopedPointer<GroupComponent> paramContainer;
	ScopedPointer<Slider> sb;
	ScopedPointer<ToggleButton> showVideo;
    ScopedPointer<Label> leftRightLbl;
    ScopedPointer<BoostingComponent> boosty;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PosTrackerCanvas);

};

class BoostingComponent :  public Component
{
public:
	BoostingComponent()
	{
		iterations = new Slider("iterations");
		iterations->setColour(Slider::backgroundColourId, Colours::yellow);
		iterations->setRange(0.0,20.0,1.0);
		iterations->setValue(3.0);
		addAndMakeVisible(iterations);

		nClassifiers = new Slider("nClassifiers");
		nClassifiers->setColour(Slider::backgroundColourId, Colours::yellow);
		nClassifiers->setRange(0.0,20.0,1.0);
		nClassifiers->setValue(3.0);
		addAndMakeVisible(nClassifiers);

		sampleOverlap = new Slider("sampleOverlap");
		sampleOverlap->setColour(Slider::backgroundColourId, Colours::yellow);
		sampleOverlap->setRange(0.0,30.0,0.5);
		sampleOverlap->setValue(2.0);
		addAndMakeVisible(sampleOverlap);

		searchFactor = new Slider("searchFactor");
		searchFactor->setColour(Slider::backgroundColourId, Colours::yellow);
		searchFactor->setRange(0.0,30.0,0.5);
		searchFactor->setValue(2.0);
		addAndMakeVisible(searchFactor);
	}
	~BoostingComponent() {};
	void paint(Graphics & g) override {
		g.fillAll(Colours::darkgrey);
	}
	void resized() override
    {
        auto area = getLocalBounds();


        auto contentItemHeight = 24;
        iterations->setBounds       (area.removeFromTop (contentItemHeight));
        nClassifiers->setBounds (area.removeFromTop (contentItemHeight));
        sampleOverlap->setBounds      (area.removeFromTop (contentItemHeight));
        searchFactor->setBounds     (area.removeFromTop (contentItemHeight));
    }
private:
	ScopedPointer<Slider> iterations;
	ScopedPointer<Slider> nClassifiers;
	ScopedPointer<Slider> sampleOverlap;
	ScopedPointer<Slider> searchFactor;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoostingComponent);
};
#endif