// NUI: Async task system implementation
// Thread pool + main thread dispatch queue.

#include "core/async.h"
#include "core/log.h"

#include <algorithm>

namespace nui {

// ── Future<T> dispatch ──────────────────────────────────────────

template<typename T>
void Future<T>::DispatchCallback() {
    // Queue callback to run on main thread
    auto cb = m_callback;
    auto result = m_result;
    Async::DispatchOnMainThread([cb, result]() {
        cb(result);
    });
}

void Future<void>::DispatchCallback() {
    auto cb = m_callback;
    Async::DispatchOnMainThread([cb]() {
        cb();
    });
}

// ── ThreadPool ──────────────────────────────────────────────────

ThreadPool::ThreadPool() {
    size_t numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
    NUI_LOG("[NUI] ThreadPool: starting %zu worker threads\n", numThreads);

    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_condition.wait(lock, [this]() {
                        return m_stop.load() || !m_tasks.empty();
                    });
                    if (m_stop.load() && m_tasks.empty()) return;
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    m_stop.store(true);
    m_condition.notify_all();
    for (auto& w : m_workers) {
        if (w.joinable()) w.join();
    }
}

void ThreadPool::Submit(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(std::move(task));
    }
    m_condition.notify_one();
}

ThreadPool& ThreadPool::Get() {
    static ThreadPool instance;
    return instance;
}

// ── Async main thread queue ─────────────────────────────────────

static std::mutex                        s_mainMutex;
static std::queue<std::function<void()>> s_mainQueue;

void Async::DispatchOnMainThread(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(s_mainMutex);
    s_mainQueue.push(std::move(callback));
}

void Async::ProcessMainThreadQueue() {
    // Swap queue under lock to minimize lock hold time
    std::queue<std::function<void()>> local;
    {
        std::lock_guard<std::mutex> lock(s_mainMutex);
        local.swap(s_mainQueue);
    }
    while (!local.empty()) {
        local.front()();
        local.pop();
    }
}

// Explicit template instantiations for common types
// (needed because implementations are in .cpp, not .h)
template class Future<int>;
template class Future<float>;
template class Future<std::string>;
template class Future<bool>;

} // namespace nui
