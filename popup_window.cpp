#include "popup_window.h"
#include "ui_popup_window.h"

#include <QKeyEvent>

popup_window::popup_window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::popup_window)
{
    ui->setupUi(this);
}

void popup_window::keyPressEvent(QKeyEvent* e)
{
    if (e->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(e);
    }
}

popup_window::~popup_window() = default;