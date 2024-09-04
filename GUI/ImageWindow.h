#pragma once
#include <QDialog>

// The window opened on the "image" button.
// Displays the information about the image currently opened.
class ImageWindow final : public QDialog {
	Q_OBJECT

public:
	ImageWindow(QWidget* parent, const QList<QPair<QString, QString>>& info);
};