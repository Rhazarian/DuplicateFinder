#ifndef FIND_DUPLICATES_H
#define FIND_DUPLICATES_H

#include <filesystem>
#include <regex>
#include <vector>
#include <stdexcept>

#include <QProgressBar>

std::vector<std::vector<std::filesystem::path>>
find_duplicates(std::filesystem::path const& dir, std::optional<std::regex> const& filter,
        std::function<void(int)> on_progress_max_determination, std::function<void(int)> on_progress_update);

#endif // FIND_DUPLICATES_H
