#pragma once
#include <QWidget>
#include <QLineEdit>

// *Idea: add a button to choose the point on the image directly

// A widget allowing to choose a point on the image
// The point can be chosen by entering its x and y
class PointChoiceEdit final : public QWidget {
	Q_OBJECT

private:
	QLineEdit* m_xEdit;
	QLineEdit* m_yEdit;

public:
	PointChoiceEdit(QWidget* parent = nullptr, uint32_t maxX = 256*256, uint32_t maxY = 256*256);

	QPoint getChosenPoint();

	// Returns true if the point is already chosen
	bool isChosen();
};