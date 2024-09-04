#pragma once
#include <QProgressDialog>

// For using, g_progressDialog must be initialized first
extern QProgressDialog* g_progressDialog;
extern bool g_isProgressDialogOpen;

void openProgressDialog(QWidget* parent, QString name, int max_val = 101);
void closeProgressDialog();

// Then the callback function can be passed on somewhere
void progressCallbackFunc(double progress);