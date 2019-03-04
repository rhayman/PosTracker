#ifndef STEREOPOSEDITOR_H_
#define STEREOPOSEDITOR_H_

#include <EditorHeaders.h>
#include "StereoPos.h"

enum class BOARDPROP {
	kWidth,
	kHeight,
	kSquareSize,
	kChessBoard,
	kCircularSymmetric,
	kCircularAsymmetric
};

class StereoPosEditor : public GenericEditor
{
public:
	StereoPosEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors);
	~StereoPosEditor() {};

	void editorShown(Label*, TextEditor& textEditorShown) {};

	void buttonEvent(Button* button);

	void updateSettings() {};
	void update() {};

	double getBoardDims(BOARDPROP);
	void setBoardDims(BOARDPROP, int);
	int getNImagesToCapture();
	void setNImagesToCapture(int);
	int getNSecondsBetweenCaptures();
	void setNSecondsBetweenCaptures(int);
	BOARDPROP getBoardType();

	void saveCapturedImages(bool val) { m_saveCapture = val; }
	bool saveCapturedImages() { return m_saveCapture; }

private:
	StereoPos * m_proc;

	ScopedPointer<TextEditor> intervalText;
	ScopedPointer<TextEditor> nImagesText;
	ScopedPointer<TextEditor> boardWidthText;
	ScopedPointer<TextEditor> boardHeightText;
	ScopedPointer<TextEditor> boardSquareSizeText;
	ScopedPointer<ComboBox> calibrationPatternCombo;
	ScopedPointer<Label> intervalLabel;
	ScopedPointer<Label> nImagesLabel;
	ScopedPointer<Label> boardWidthLabel;
	ScopedPointer<Label> boardHeightLabel;
	ScopedPointer<Label> boardSquareSizeLabel;
	ScopedPointer<Label> calibrationPatternLabel;
	ScopedPointer<UtilityButton> captureButton;
	ScopedPointer<ToggleButton> showVideoCapture;

	bool m_saveCapture = false;

	String board_width = String("11");
	String board_height = String("12");
	String square_size = String("11");

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoPosEditor);
};

#endif