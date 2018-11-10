#ifndef ERROR_POPUP_WINDOW_H
#define ERROR_POPUP_WINDOW_H

#include <QDialog>

#include <memory>

class main_window;

namespace Ui {
class error_popup_window;
}

class error_popup_window : public QDialog
{
    Q_OBJECT

    friend main_window;

public:
    explicit error_popup_window(QWidget *parent = nullptr);
    ~error_popup_window() override;

private:
    std::unique_ptr<Ui::error_popup_window> ui;
};

#endif // ERROR_POPUP_WINDOW_H
