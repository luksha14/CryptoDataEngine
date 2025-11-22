#ifndef PROCESSING_THREAD_H
#define PROCESSING_THREAD_H

#include <thread>
#include <iostream>
#include "SafeQueue.h"
#include "Persistence.h"

class ProcessingThread {
private:
    SafeQueue<TickerData>& data_queue_;
    PersistenceManager& db_manager_;
    std::thread thread_;
    bool running_ = false;
    void process_and_insert_batch(const std::vector<TickerData>& batch);
    double last_ema_value_20_ = 0.0;
    bool is_first_ema_20_ = true;
    const int EMA_PERIOD_20 = 20;
    double last_ema_value_50_ = 0.0;
    bool is_first_ema_50_ = true;
    const int EMA_PERIOD_50 = 50;
    void process_data_loop();

public:
    ProcessingThread(SafeQueue<TickerData>& queue, PersistenceManager& db_mgr);
    ~ProcessingThread();

    void start_thread();
    void stop_thread();
    double calculate_ema(double current_price, int period);
};

#endif // PROCESSING_THREAD_H