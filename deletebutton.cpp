#include "deletebutton.h"

void DeleteButton::setMainWindow(MainWindow *const mainWindow)
{
    _mainWindow = mainWindow;
    setStyleSheet("icon: url(images/deleteBtn.png); padding: 4px; margin: 0px;");
    connect(this, &DeleteButton::released, this, &DeleteButton::onDeleteButtonClicked);
}

void DeleteButton::onDeleteButtonClicked()
{
    emit _mainWindow->removeLineSignal(this);
}
