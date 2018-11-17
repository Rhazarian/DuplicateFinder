#include <utility>

#include "main_window.h"
#include "ui_main_window.h"

#include "popups/ui_popup_window.h"
#include "popups/ui_error_popup_window.h"
#include "popups/ui_delete_popup_window.h"

#include "duplicate_finder.h"

#include <QDir>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QStandardItemModel>

#include <regex>
#include <filesystem>

#include "find_duplicates.h"

namespace fs = std::filesystem;

Q_DECLARE_METATYPE(std::vector<std::vector<std::filesystem::path>>);

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window()),
    popup(new popup_window(this)),
    error_popup(new error_popup_window(this)),
    delete_popup(new delete_popup_window(this))
{
    ui->setupUi(this);

    QPalette palette;
    palette.setColor(QPalette::Base, ui->changeDirButton->palette().button().color());
    ui->currentDir->setPalette(palette);
    ui->currentDir->setText(QDir::currentPath());
    ui->filterRegex->setPalette(palette);
    ui->treeWidget->setPalette(palette);
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeWidget->setUniformRowHeights(true);

    popup->setWindowModality(Qt::WindowModality::WindowModal);
    error_popup->setWindowModality(Qt::WindowModality::WindowModal);
    delete_popup->setWindowModality(Qt::WindowModality::WindowModal);

    error_popup->ui->textBrowser->setPalette(palette);

    delete_popup->ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

    qRegisterMetaType<std::vector<std::vector<std::filesystem::path>>>();

    connect(ui->scanButton, &QPushButton::clicked, this, &main_window::scan);
    connect(ui->filterRegex, &QLineEdit::textEdited, this, &main_window::validate_regex);
    connect(ui->filterFlag, &QCheckBox::stateChanged, this, &main_window::filter_state_changed);
    connect(ui->currentDir, &QLineEdit::textEdited, this, &main_window::validate_dir);
    connect(ui->changeDirButton, &QPushButton::clicked, this, &main_window::change_dir);
    connect(ui->expandAllButton, &QPushButton::clicked, this, &main_window::expand_all);
    connect(ui->collapseAllButton, &QPushButton::clicked, this, &main_window::collapse_all);
    connect(ui->autoselectButton, &QPushButton::clicked, this, &main_window::autoselect);
    connect(ui->deleteButton, &QPushButton::clicked, this, &main_window::delete_selected);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &main_window::validate_selection);

    connect(popup->ui->pushButton, &QPushButton::clicked, this, &main_window::request_cancel_scan);
}

main_window::~main_window() = default;

void main_window::scan()
{
    scanning_thread = new QThread();
    auto* worker = new duplicate_finder(ui->currentDir->text().toStdString(),
            ui->filterFlag->checkState() ? std::make_optional(std::regex(ui->filterRegex->text().toStdString())) : std::nullopt);
    worker->moveToThread(scanning_thread);
    connect(worker, &duplicate_finder::error, this, &main_window::scan_error);
    connect(scanning_thread, &QThread::started, worker, &duplicate_finder::process);
    connect(worker, &duplicate_finder::update_bar_max, this, &main_window::set_bar_max);
    connect(worker, &duplicate_finder::update_bar_progress, this, &main_window::set_bar_progress);
    connect(worker, &duplicate_finder::finished, this, &main_window::finish_scan);
    connect(worker, &duplicate_finder::finished, scanning_thread, &QThread::quit);
    connect(worker, &duplicate_finder::finished, worker, &duplicate_finder::deleteLater);
    connect(scanning_thread, &QThread::finished, scanning_thread, &QThread::deleteLater);
    scanning_thread->start();
    popup->ui->progressBar->setValue(0);
    popup->ui->pushButton->setEnabled(true);
    popup->open();
}

void main_window::request_cancel_scan() {
    popup->ui->pushButton->setEnabled(false);
    scanning_thread->requestInterruption();
}

void main_window::finish_scan(std::vector<std::vector<std::filesystem::path>> duplicates) {
    popup->close();
    ui->treeWidget->setUpdatesEnabled(false);
    ui->treeWidget->clear();
    for (size_t i = 0; i < duplicates.size(); ++i) {
        auto* top_item = new QTreeWidgetItem();
        fs::path& head = duplicates[i][0];
        top_item->setText(0, QString("Group %1 (%2), files: %3, size of each: %4 bytes").
        arg(std::to_string(i + 1).c_str(), head.filename().c_str(),
                std::to_string(duplicates[i].size()).c_str(), std::to_string(fs::file_size(head)).c_str()));
        top_item->setFlags(top_item->flags() & ~Qt::ItemIsSelectable);
        ui->treeWidget->addTopLevelItem(top_item);
        for (auto& duplicate : duplicates[i]) {
            auto* item = new QTreeWidgetItem();
            item->setText(0, duplicate.c_str());
            top_item->addChild(item);
        }
    }
    ui->treeWidget->setUpdatesEnabled(true);
    bool enable = !duplicates.empty();
    ui->expandAllButton->setEnabled(enable);
    ui->collapseAllButton->setEnabled(enable);
    ui->autoselectButton->setEnabled(enable);
    ui->deleteButton->setEnabled(false);
}

void main_window::error(QString err)
{
    error_popup->ui->textBrowser->setText(err);
    error_popup->open();
}

void main_window::scan_error(QString err)
{
    finish_scan({});
    error(std::move(err));
}

void main_window::expand_all()
{
    ui->treeWidget->expandAll();
}

void main_window::collapse_all()
{
    ui->treeWidget->collapseAll();
}

void main_window::autoselect()
{
    ui->treeWidget->setUpdatesEnabled(false);
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        auto* top_item = ui->treeWidget->topLevelItem(i);
        if (top_item->isExpanded()) {
            if (top_item->childCount() > 0) {
                top_item->child(0)->setSelected(false);
                for (int j = 1; j < top_item->childCount(); ++j) {
                    top_item->child(j)->setSelected(true);
                }
            }
        } else {
            for (int j = 0; j < top_item->childCount(); ++j) {
                top_item->child(j)->setSelected(false);
            }
        }
    }
    ui->treeWidget->setUpdatesEnabled(true);
}

void main_window::delete_selected()
{
    try {
        delete_popup->ui->label->setText(
                QString("Are you sure to delete %1 files?").arg(std::to_string(ui->treeWidget->selectedItems().size()).c_str()));
        if (delete_popup->exec() == QDialog::Accepted) {
            for (auto item : ui->treeWidget->selectedItems()) {
                fs::remove(item->text(0).toStdString());
                item->parent()->removeChild(item);
            }
        }
    } catch (std::exception& ex) {
        error(ex.what());
    }
}

void main_window::set_bar_max(int max)
{
    popup->ui->progressBar->setMaximum(max);
}

void main_window::set_bar_progress(int progress)
{
    popup->ui->progressBar->setValue(progress);
}

void main_window::change_dir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"));
    if (!dir.isEmpty()) {
        ui->currentDir->setText(dir);
    }
    validate_dir();
}

void main_window::validate()
{
    bool enable = is_dir_valid && (!ui->filterFlag->checkState() || is_regex_valid);
    ui->scanButton->setEnabled(enable);
}

void main_window::validate_dir()
{
    QFileInfo dir(ui->currentDir->text());
    is_dir_valid = dir.exists() && dir.isDir();
    validate();
}

void main_window::validate_regex()
{
    is_regex_valid = true;
    try {
        std::regex test(ui->filterRegex->text().toStdString());
    } catch (std::regex_error&) {
        is_regex_valid = false;
    }
    validate();
}

void main_window::validate_selection()
{
    ui->deleteButton->setEnabled(!ui->treeWidget->selectedItems().empty());
}

void main_window::filter_state_changed()
{
    bool filterEnabled = ui->filterFlag->checkState();
    ui->filterRegex->setEnabled(filterEnabled);
    ui->filterRegexLabel->setEnabled(filterEnabled);
    validate();
}
