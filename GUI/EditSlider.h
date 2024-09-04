#pragma once
#include <QLineEdit>
#include <QSlider>
#include <QBoxLayout>

// A pair of a slider and a lineedit to pick some value
class EditSlider final : public QWidget {
	Q_OBJECT

private:
	QVBoxLayout* m_layout;
	QSlider* m_slider;
	QLineEdit* m_lineEdit;

public:
	EditSlider(QWidget* parent, Qt::Orientation orientation, int valMin, int valMax, int step);

	void setValue(int value);
	int getValue();

signals:
	void valueChanged(int);

private slots:
	void lineEditValueChangedSlot(const QString& newVal);
	void sliderValueChangedSlot(int newVal);
};