#ifndef DELETEBUTTON_H
#define DELETEBUTTON_H

#include <QPushButton>
#include "mainwindow.h"

class MainWindow;

class DeleteButton: public QPushButton
{
public:
    void setMainWindow(MainWindow *const mainWindow);
    void onDeleteButtonClicked();

private:
    MainWindow *_mainWindow;
};

#endif // DELETEBUTTON_H
