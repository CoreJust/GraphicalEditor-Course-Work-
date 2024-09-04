#include "ProgressDialog.h"

QProgressDialog* g_progressDialog = nullptr;
bool g_isProgressDialogOpen = false;

void openProgressDialog(QWidget* parent, QString name, int max_val) {
	assert(!g_isProgressDialogOpen && "Cannot open progress dialog twice");

    g_isProgressDialogOpen = true;
    g_progressDialog = new QProgressDialog(name, "No cancel", 0, max_val, parent);
    g_progressDialog->setWindowModality(Qt::WindowModal);
    g_progressDialog->setCancelButton(0);
    g_progressDialog->setWindowFlags(g_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
    g_progressDialog->setAutoClose(false);
    g_progressDialog->setValue(0);
    g_progressDialog->setMinimumDuration(500);
    g_progressDialog->show();
}

void closeProgressDialog() {
    assert(g_isProgressDialogOpen && "Cannot close progress dialog if it is not opened");
    
    g_isProgressDialogOpen = false;

    if (!g_progressDialog->wasCanceled()) {
        g_progressDialog->close();
    }
}

void progressCallbackFunc(double progress) {
	g_progressDialog->setValue(progress);
}
