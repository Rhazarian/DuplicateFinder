#ifndef LIBDUPFINDER_H
#define LIBDUPFINDER_H

#include <filesystem>
#include <regex>
#include <vector>
#include <stdexcept>

#include <QProgressBar>

std::vector<std::vector<std::filesystem::path>>
find_duplicates(std::filesystem::path const& dir, std::optional<std::regex> const& filter,
        std::function<void(int)> on_progress_max, std::function<void(int)> on_progress_update);

#endif // LIBDUPFINDER_H
