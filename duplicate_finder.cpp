#include "duplicate_finder.h"

#include "find_duplicates.h"

namespace fs = std::filesystem;

duplicate_finder::duplicate_finder(fs::path path, std::optional<std::regex> filter)
        :path(std::move(path)),
         filter(std::move(filter)) { }

duplicate_finder::~duplicate_finder() = default;

void duplicate_finder::process()
{
    try {
        auto duplicates = find_duplicates(path, filter,
                [&](int max) {emit_update_bar_max(max);}, [&](int progress) {emit_update_bar_progress(progress); });
        emit finished(duplicates);
    } catch (std::exception& ex) {
        emit error(ex.what());
    }
}

void duplicate_finder::emit_update_bar_max(int max) {
    emit update_bar_max(max);
}

void duplicate_finder::emit_update_bar_progress(int progress) {
    emit update_bar_progress(progress);
}