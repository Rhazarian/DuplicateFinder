#include "find_duplicates.h"

#include <map>
#include <fstream>
#include <iomanip>
#include <string>
#include <set>
#include <thread>
#include <iostream>
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
    const auto thread_count = std::min(std::thread::hardware_concurrency(), 4u);
    std::vector<std::thread> threads;
    boost::lockfree::queue<fs::path*> paths(4);
    std::atomic_bool work_given;
    std::atomic_bool finished;
    std::vector<std::atomic_bool> work_done(thread_count);
    std::condition_variable work;
    std::condition_variable done;
    std::mutex work_wait_mtx;
    std::mutex wait_mtx;
    std::mutex mtx;
    std::exception_ptr ex_ptr;
    std::map<std::string, std::vector<fs::path>> hash_buckets;
    std::vector<std::vector<fs::path>> duplicates;
    try {
        if (!fs::is_directory(dir)) {
            throw std::invalid_argument("Provided path should refer to a directory");
        }

        auto cancellation_point = [thread = QThread::currentThread()]() {
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
                cancellation_point();
                fin.read(buffer.data(), buffer.size());
                gcount = static_cast<int>(fin.gcount());
                hash.addData(buffer.data(), gcount);
            }
            while (gcount > 0);
            return hash.result().toStdString();
        };

        int file_count = 0;

        auto consumer = [&](int i) {
            while (!finished) {
                try {
                    {
                        std::unique_lock<std::mutex> lk(work_wait_mtx);
                        work.wait(lk, [&work_given, &work_done, i] { return work_given == true && !work_done[i]; });
                    }
                    fs::path* path_ptr;
                    while (paths.pop(path_ptr) && !ex_ptr) {
                        fs::path path = *path_ptr;
                        auto hash = get_sha256hash(path);
                        std::lock_guard<std::mutex> lg(mtx);
                        hash_buckets[hash].emplace_back(std::move(path));
                        ++file_count;
                        cancellation_point();
                        on_progress_update(file_count);
                    }
                }
                catch (...) {
                    std::lock_guard<std::mutex> lg(mtx);
                    if (!ex_ptr) {
                        ex_ptr = std::current_exception();
                    }
                }
                std::lock_guard<std::mutex> lg(wait_mtx);
                work_done[i] = true;
                if (std::all_of(work_done.begin(), work_done.end(), [](std::atomic_bool& b) {
                    return b == true;
                })) {
                    done.notify_one();
                }
            }
        };
        for (auto i = 0; i < thread_count; ++i) {
            threads.emplace_back(consumer, i);
        }

        std::unordered_map<uintmax_t, std::vector<fs::path>> size_buckets;
        for (auto& path : fs::recursive_directory_iterator(dir)) {
            cancellation_point();
            if (!path.is_regular_file()) {
                continue;
            }
            if (!filter.has_value() || std::regex_match(path.path().string(), *filter)) {
                size_buckets[path.file_size()].emplace_back(path.path());
                ++file_count;
            }
        }
        on_progress_max(file_count);

        file_count = 0;
        for (auto& size_bucket : size_buckets) {
            cancellation_point();
            if (auto count = size_bucket.second.size(); count > 1) {
                hash_buckets.clear();
                paths.reserve(count);
                for (auto& path : size_bucket.second) {
                    paths.push(&path);
                }

                {
                    std::lock_guard<std::mutex> lg(work_wait_mtx);
                    std::for_each(work_done.begin(), work_done.end(), [](std::atomic_bool& b) {
                        b = false;
                    });
                    work_given = true;
                    work.notify_all();
                }

                std::unique_lock<std::mutex> lk(wait_mtx);
                done.wait(lk, [&work_done]() {
                    return std::all_of(work_done.begin(), work_done.end(), [](std::atomic_bool& b) {
                        return b == true;
                    });
                });
                work_given = false;
                if (ex_ptr) {
                    std::rethrow_exception(ex_ptr);
                }

                for (auto& bucket : hash_buckets) {
                    cancellation_point();
                    if (bucket.second.size() > 1) {
                        std::vector<fs::path> files;
                        for (auto& file : bucket.second) {
                            files.push_back(file);
                        }
                        duplicates.push_back(std::move(files));
                    }
                }
            }
        }
        cancellation_point();
    }
    catch (...) {
        duplicates.clear();
        ex_ptr = std::current_exception();
    }

    {
        std::lock_guard<std::mutex> lg(work_wait_mtx);
        finished = true;
        std::for_each(work_done.begin(), work_done.end(), [](std::atomic_bool& b) {
            b = false;
        });
        work_given = true;
        work.notify_all();
    }

    std::unique_lock<std::mutex> lk(wait_mtx);
    done.wait(lk, [&work_done]() {
        return std::all_of(work_done.begin(), work_done.end(), [](std::atomic_bool& b) {
            return b == true;
        });
    });

    for (auto& thread : threads) {
        thread.join();
    }
    try {
        if (ex_ptr) {
            std::rethrow_exception(ex_ptr);
        }
    } catch (cancellation_exception& ex) {
        // No operations.
    }
    return duplicates;
}