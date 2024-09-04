#include "HelpWindow.h"
#include <QFile>
#include <QHBoxLayout>
#include <QMessageBox>

constexpr uint32_t MINIMUM_ABOUT_WIDTH = 320;
constexpr uint32_t MINIMUM_ABOUT_HEIGHT = 240;
constexpr uint32_t BASE_ABOUT_WIDTH = 600;
constexpr uint32_t BASE_ABOUT_HEIGHT = 600;

const char* ABOUT_HTML_FILE_PATH = "res/html/about.html";

// Color scheme: background #fd6d0d, text #ffefcc

HelpWindow::HelpWindow(QWidget* parent)
	: QDialog(parent) {
    setWindowTitle(tr("About"));
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    setMinimumSize(MINIMUM_ABOUT_WIDTH, MINIMUM_ABOUT_HEIGHT);
    resize(BASE_ABOUT_WIDTH, BASE_ABOUT_HEIGHT);

	m_textBrowser = new QTextBrowser(this);
    m_textBrowser->setOpenLinks(false);
    m_textBrowser->setOpenExternalLinks(false);
    m_textBrowser->setAcceptRichText(true);

    connect(m_textBrowser, SIGNAL(anchorClicked(QUrl)), this, SLOT(onAnchorClicked(QUrl)));

    QHBoxLayout* boxLayout = new QHBoxLayout(this);
    boxLayout->addWidget(m_textBrowser);
}

utils::Result<Void> HelpWindow::openHtml(const QString& fileName) {
    QFile htmlFile(fileName);
    if (!htmlFile.open(QIODevice::ReadOnly)) {
        return utils::Failure("Failed to open html: " + fileName);
    }
    
    QTextStream stream(&htmlFile);
    QString html = stream.readAll();
    m_textBrowser->setText(html);

    return utils::Success();
}

void HelpWindow::onAnchorClicked(const QUrl& link) {
    if (link.isRelative()) {
        auto result = openHtml("res/html/" + link.toString());
        if (!result.isOk()) {
            QMessageBox msg;
            msg.warning(this, "Error", result.error());
            msg.show();
        }
    }
}
