#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include <memory>
#include <filesystem>

#include "popups/popup_window.h"
#include "popups/error_popup_window.h"
#include "popups/delete_popup_window.h"

namespace Ui {
class main_window;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window() override;

private:
    std::unique_ptr<Ui::main_window> ui;

    std::unique_ptr<popup_window> popup;
    std::unique_ptr<error_popup_window> error_popup;
    std::unique_ptr<delete_popup_window> delete_popup;

    QThread* scanning_thread = nullptr;

    bool is_dir_valid = true;
    bool is_regex_valid = true;

    void validate();

private slots:
    void scan();
    void request_cancel_scan();
    void change_dir();
    void validate_dir();
    void validate_regex();
    void filter_state_changed();
    void scan_error(QString err);
    void finish_scan(std::vector<std::vector<std::filesystem::path>> duplicates);
    void expand_all();
    void collapse_all();
    void autoselect();
    void delete_selected();
    void set_bar_max(int max);
    void set_bar_progress(int progress);
    void validate_selection();

private:
    void error(QString err);

};

#endif // MAIN_WINDOW_H
