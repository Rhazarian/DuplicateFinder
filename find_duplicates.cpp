#include "find_duplicates.h"

#include <map>
#include <fstream>
#include <iomanip>
#include <string>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include <boost/lockfree/queue.hpp>

#include <QtCore>

namespace fs = std::filesystem;

namespace {

    struct cancellation_exception : std::exception {
    };

}

std::vector<std::vector<fs::path>>
find_duplicates(fs::path const& dir, std::optional<std::regex> const& filter,
        std::function<void(int)> on_progress_max, std::function<void(int)> on_progress_update)
{
    try {
        if (!fs::is_directory(dir)) {
            throw std::invalid_argument("Provided path should refer to a directory");
        }
        auto qcancellation_point = [thread = QThread::currentThread()]() {
            if (thread->isInterruptionRequested()) {
                throw cancellation_exception();
            }
        };
        auto get_sha256hash = [&](fs::path const& path) {
            std::array<char, 8192> buffer{};
            std::ifstream fin(path, std::ios::binary);
            if (!fin) {
                throw std::runtime_error("Could not get hash of \"" + path.string() + "\"");
            }
            QCryptographicHash hash(QCryptographicHash::Sha256);
            int gcount = 0;
            do {
                qcancellation_point();
                fin.read(buffer.data(), buffer.size());
                gcount = static_cast<int>(fin.gcount());
                hash.addData(buffer.data(), gcount);
            }
            while (gcount > 0);
            return hash.result().toStdString();
        };
        std::unordered_multimap<uintmax_t, fs::path> size_buckets;
        std::unordered_set<uintmax_t> keys;
        int file_count = 0;
        for (auto& path : fs::recursive_directory_iterator(dir)) {
            qcancellation_point();
            if (!path.is_regular_file()) {
                continue;
            }
            if (!filter.has_value() || std::regex_match(path.path().string(), *filter)) {
                size_buckets.emplace(path.file_size(), path.path());
                keys.insert(path.file_size());
                ++file_count;
            }
        }
        on_progress_max(file_count);
        file_count = 0;
        std::vector<std::vector<fs::path>> duplicates;
        for (auto key : keys) {
            qcancellation_point();
            if (auto count = size_buckets.count(key); count > 1) {
                std::multimap<std::string, fs::path> hash_buckets;
                std::set<std::string> hash_keys;
                boost::lockfree::queue<fs::path*> paths(count);
                auto range = size_buckets.equal_range(key);
                for (auto& it = range.first; it != range.second; ++it) {
                    paths.push(&it->second);
                }
                std::mutex mtx;
                std::exception_ptr ex_ptr;
                auto consumer = [&]() {
                    try {
                        fs::path* path_ptr;
                        while (paths.pop(path_ptr) && !ex_ptr) {
                            fs::path path = *path_ptr;
                            auto hash = get_sha256hash(path);
                            std::lock_guard<std::mutex> lg(mtx);
                            hash_buckets.emplace(hash, std::move(path));
                            hash_keys.insert(hash);
                            ++file_count;
                            qcancellation_point();
                            on_progress_update(file_count);
                        }
                    }
                    catch (...) {
                        std::lock_guard<std::mutex> lg(mtx);
                        if (!ex_ptr) {
                            ex_ptr = std::current_exception();
                        }
                    }
                };
                std::vector<std::thread> threads;
                auto thread_count = std::min(count, static_cast<decltype(count)>(4));
                for (auto i = 0; i < thread_count; ++i) {
                    threads.emplace_back(consumer);
                }
                for (auto& thread : threads) {
                    thread.join();
                }
                if (ex_ptr) {
                    std::rethrow_exception(ex_ptr);
                }
                for (auto& hash_key : hash_keys) {
                    qcancellation_point();
                    auto bucket_size = hash_buckets.count(hash_key);
                    if (bucket_size > 1) {
                        std::vector<fs::path> files;
                        auto hash_range = hash_buckets.equal_range(hash_key);
                        for (auto& it = hash_range.first; it != hash_range.second; ++it) {
                            files.push_back(it->second);
                        }
                        duplicates.push_back(std::move(files));
                    }
                }
            }
        }
        qcancellation_point();
        return duplicates;
    }
    catch (cancellation_exception& ex) {
        // No operations.
    }
    return {};
}