#pragma once
#include <QDialog>
#include "ColorLabel.h"
#include "EditSlider.h"

// The window opened on the "help" button.
// Displays the html files with information about the program and its usage.
// Although there is QColorDialog, I like mine more
class ColorChoiceWindow final : public QDialog {
	Q_OBJECT

private:
	// Label for previewing chosen color
	ColorLabel* m_colorPreview;

	// Color sliders
	EditSlider* m_rSlider;
	EditSlider* m_gSlider;
	EditSlider* m_bSlider;
	EditSlider* m_aSlider;

	// Standard color set
	ColorLabel* m_colorSetLabels[16];

public:
	ColorChoiceWindow(QWidget* parent, Color color);

	Color getChosenColor();

	// Opens up the color choice window and on successful color choice changes the -color-
	// On cancel does nothing to the -color-
	// Returns whether the color was changed
	static bool chooseColor(QWidget* parent, Color &color);

private slots:
	void rChangedSlot(int val);
	void gChangedSlot(int val);
	void bChangedSlot(int val);
	void aChangedSlot(int val);
	void colorSetClickedSlot(int id);
};