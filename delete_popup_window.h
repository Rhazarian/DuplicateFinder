#ifndef DELETE_POPUP_WINDOW_H
#define DELETE_POPUP_WINDOW_H

#include <QDialog>

#include <memory>

class main_window;

namespace Ui {
class delete_popup_window;
}

class delete_popup_window : public QDialog
{
    Q_OBJECT

    friend main_window;

public:
    explicit delete_popup_window(QWidget *parent = nullptr);
    ~delete_popup_window() override;

private:
    std::unique_ptr<Ui::delete_popup_window> ui;
};

#endif // DELETE_POPUP_WINDOW_H
