#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include "TickerData.h" 

/**
 * @brief Thread-safe queue for TickerData. 
 * This is crucial for safely passing data between the Data Ingestor thread 
 * and the Processing thread without data corruption.
 */
template <typename T>
class SafeQueue {
private:
    std::queue<TickerData> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_flag_ = false;

public:
    // Destructor is needed to unblock waiting threads upon shutdown
    ~SafeQueue() {
        stop_flag_ = true;
        condition_.notify_all(); 
    }

    /**
     * @brief Pushes data onto the queue.
     * @param data The TickerData object to push.
     */
    void push(const TickerData& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(data);
        condition_.notify_one(); // Notify one waiting thread that data is available
    }

    /**
     * @brief Pops data from the queue, blocking if the queue is empty.
     * @return std::optional<TickerData> The data, or empty if stop_flag is set.
     */
    std::optional<TickerData> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until the queue is not empty OR the stop flag is set
        condition_.wait(lock, [this] {
            return !queue_.empty() || stop_flag_;
        });

        if (stop_flag_ && queue_.empty()) {
            return std::nullopt; // System is shutting down
        }

        // Extract the data
        TickerData data = std::move(queue_.front());
        queue_.pop();
        return data;
    }

    /**
     * @brief Pokušava izvući element iz reda bez blokiranja.
     * @return std::optional<T> Element ako postoji, inače std::nullopt.
     */
    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Jednostavno provjeri red, bez čekanja na condition_variable
        if (queue_.empty()) {
            // Nema elementa, vrati prazan optional
            return std::nullopt; 
        }
        // Stvori i vrati std::optional koji sadrži izvađeni element
        T value = queue_.front();
        queue_.pop();
        return std::optional<T>(value); // Ispravan povratak
    }
};

#endif 