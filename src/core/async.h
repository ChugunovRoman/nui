#pragma once
// NUI: Async task system
// Lightweight thread pool + Future<T> with main thread dispatch.
//
// Usage:
//   Async::Run([]() { return heavyComputation(); })
//       ->Then([](int result) { label->SetText(std::to_string(result)); });

#include <functional>
#include <memory>
#include <future>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace nui {

class Application;

// ── Future<T> — result of an async operation ────────────────────
template<typename T>
class Future {
public:
    using Ptr = std::shared_ptr<Future<T>>;

    // Register a callback that runs on the main thread when the result is ready.
    // If the result is already ready, the callback is queued immediately.
    void Then(std::function<void(T)> callback) {
        m_callback = std::move(callback);
        if (m_ready) {
            DispatchCallback();
        }
    }

    // Check if the result is ready
    bool IsReady() const { return m_ready; }

    // Get the result (blocks until ready)
    T Get() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_ready; });
        return m_result;
    }

    // Internal: set the result and trigger callback
    void SetValue(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_result = std::move(value);
            m_ready = true;
        }
        m_cv.notify_all();
        if (m_callback) {
            DispatchCallback();
        }
    }

private:
    void DispatchCallback();

    T m_result{};
    std::function<void(T)> m_callback;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_ready = false;
};

// Specialization for void
template<>
class Future<void> {
public:
    using Ptr = std::shared_ptr<Future<void>>;

    void Then(std::function<void()> callback) {
        m_callback = std::move(callback);
        if (m_ready) DispatchCallback();
    }

    bool IsReady() const { return m_ready; }

    void Get() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_ready; });
    }

    void SetDone() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_ready = true;
        }
        m_cv.notify_all();
        if (m_callback) DispatchCallback();
    }

private:
    void DispatchCallback();

    std::function<void()> m_callback;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_ready = false;
};

// ── ThreadPool — simple worker pool ─────────────────────────────
class ThreadPool {
public:
    static ThreadPool& Get(); // Singleton

    // Submit a task to the pool
    void Submit(std::function<void()> task);

    // Get number of worker threads
    size_t GetThreadCount() const { return m_workers.size(); }

    ~ThreadPool();

private:
    ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    std::vector<std::thread>        m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex                       m_mutex;
    std::condition_variable          m_condition;
    std::atomic<bool>                m_stop{false};
};

// ── Async — static API for running tasks ────────────────────────
class Async {
public:
    // Run a function on a background thread. Returns a Future<T>.
    // The Future::Then() callback runs on the main thread.
    template<typename F>
    static auto Run(F&& func) -> std::shared_ptr<Future<decltype(func())>> {
        using ReturnType = decltype(func());
        auto future = std::make_shared<Future<ReturnType>>();

        ThreadPool::Get().Submit([future, func = std::forward<F>(func)]() {
            if constexpr (std::is_void_v<ReturnType>) {
                func();
                future->SetDone();
            } else {
                future->SetValue(func());
            }
        });

        return future;
    }

    // Dispatch a callback on the main thread (from any thread)
    static void DispatchOnMainThread(std::function<void()> callback);

    // Process all pending main thread callbacks (called by Application::Run)
    static void ProcessMainThreadQueue();
};

} // namespace nui
