#ifndef POPUP_WINDOW_H
#define POPUP_WINDOW_H

#include <QDialog>
#include <QProgressBar>

#include <memory>

class main_window;

namespace Ui {
class popup_window;
}

class popup_window : public QDialog
{
    Q_OBJECT

    friend main_window;

public:
    explicit popup_window(QWidget *parent = nullptr);
    ~popup_window() override;

private:
    std::unique_ptr<Ui::popup_window> ui;
};

#endif // POPUP_WINDOW_H
