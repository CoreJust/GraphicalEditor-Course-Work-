#include "ColorLabel.h"
#include "ColorChoiceWindow.h"

ColorLabel::ColorLabel(QWidget* parent, bool openColorChoiceOnClick, int id)
	: QLabel(parent), m_openColorChoiceOnClick(openColorChoiceOnClick), m_id(id) {
	setColor(Color::Black);
}

void ColorLabel::setColor(Color color) {
	m_color = color;
	QString colorString = "rgba(" 
		+ QString::number(color.r()) + ", "
		+ QString::number(color.g()) + ", "
		+ QString::number(color.b()) + ", "
		+ QString::number(color.a()) + ")";

	setStyleSheet("QLabel { color: " + colorString + "; background-color: " + colorString + "; border: 2px solid white; }");

	emit colorChanged(color);
}

Color ColorLabel::getColor() const {
	return m_color;
}

void ColorLabel::setOpenColorChoiceOnClick(bool value) {
	m_openColorChoiceOnClick = value;
}

void ColorLabel::mousePressEvent(QMouseEvent* event) {
	if (m_openColorChoiceOnClick) {
		Color color = m_color;
		if (ColorChoiceWindow::chooseColor(this, color)) {
			setColor(color);
		}
	}

	emit clicked(m_id);
}
