#include "error_popup_window.h"
#include "ui_error_popup_window.h"

error_popup_window::error_popup_window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::error_popup_window)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &QDialog::accept);
}

error_popup_window::~error_popup_window() = default;