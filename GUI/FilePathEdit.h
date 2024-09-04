#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

// A pair of a line edit and a button with QFileDialog
// Allows to get a file path
class FilePathEdit final : public QWidget {
	Q_OBJECT

private:
	QLineEdit* m_lineEdit = nullptr;
	QPushButton* m_fileDialogButton = nullptr;

	QString m_fileTypes;
	bool m_isDirectoryMode = true;

public:
	FilePathEdit(QWidget* parent = nullptr, const QString& defaultPath = "", const QString& fileTypes = "", bool isDirectoryMode = true);

	QString getPath();

private slots:
	void fileDialogButtonClickedSlot();
};