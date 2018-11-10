#ifndef DUPLICATE_FINDER_H
#define DUPLICATE_FINDER_H

#include <QtCore>

#include <filesystem>
#include <regex>

#include <boost/thread.hpp>

class duplicate_finder : public QObject
{
    Q_OBJECT

public:
    duplicate_finder(std::filesystem::path path, std::optional<std::regex> filter);
    ~duplicate_finder() override;

public slots:
    void process();

signals:
    void finished(std::vector<std::vector<std::filesystem::path>> duplicates);
    void error(QString err);
    void update_bar_max(int max);
    void update_bar_progress(int progress);

private:
    std::filesystem::path path;
    std::optional<std::regex> filter;
    void emit_update_bar_max(int max);
    void emit_update_bar_progress(int progress);
};

#endif // DUPLICATE_FINDER
