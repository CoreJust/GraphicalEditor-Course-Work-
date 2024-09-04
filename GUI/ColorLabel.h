#pragma once
#include <QLabel>
#include "../Image/Color.h"

// The label displaying some color choice
class ColorLabel final : public QLabel {
	Q_OBJECT

private:
	Color m_color;
	int m_id = 0; // For a group of ColorLabels
	bool m_openColorChoiceOnClick = false; // Whether to open a color choice menu on click

public:
	ColorLabel(QWidget* parent = nullptr, bool openColorChoiceOnClick = false, int id = 0);

	void setColor(Color color);
	Color getColor() const;

	void setOpenColorChoiceOnClick(bool value);

signals:
	void clicked(int id);
	void colorChanged(Color newColor);

private:
	void mousePressEvent(QMouseEvent* event);
};