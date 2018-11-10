#include "popup_window.h"
#include "ui_popup_window.h"

popup_window::popup_window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::popup_window)
{
    ui->setupUi(this);
}

popup_window::~popup_window() = default;