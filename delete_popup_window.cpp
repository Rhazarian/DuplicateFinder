#include "delete_popup_window.h"
#include "ui_delete_popup_window.h"

delete_popup_window::delete_popup_window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::delete_popup_window)
{
    ui->setupUi(this);
}

delete_popup_window::~delete_popup_window() = default;