#pragma once
#include <QTextBrowser>
#include <QDialog>
#include "../Utils/Result.h"

// The window opened on the "help" button.
// Displays the html files with information about the program and its usage.
class HelpWindow final : public QDialog {
	Q_OBJECT

private:
	QTextBrowser* m_textBrowser;

public:
	HelpWindow(QWidget* parent);

	utils::Result<Void> openHtml(const QString& fileName);

private slots:
	void onAnchorClicked(const QUrl& link);
};

extern const char* ABOUT_HTML_FILE_PATH;