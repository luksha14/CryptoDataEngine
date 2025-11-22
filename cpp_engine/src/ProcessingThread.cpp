#include "../include/ProcessingThread.h"
#include "../include/Constants.h" 
#include <chrono>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <optional>

using namespace std; 


// --- Constructor / Destructor ---

ProcessingThread::ProcessingThread(SafeQueue<TickerData>& queue, PersistenceManager& db_mgr)
    : data_queue_(queue), db_manager_(db_mgr), running_(true) 
{
    // C++ threadovi se pokreću u start_thread metodi
}

ProcessingThread::~ProcessingThread() {
    stop_thread();
}

// --- Thread Control ---

void ProcessingThread::start_thread() {
    if (!thread_.joinable()) {
        running_ = true;
        thread_ = std::thread(&ProcessingThread::process_data_loop, this);
        cout << "Processing thread started." << endl;
    }
}

void ProcessingThread::stop_thread() {
    if (thread_.joinable()) {
        running_ = false;
        thread_.join();
        cout << "Processing thread stopped." << endl;
    }
}

// --- Helper: Iznos i upis batcha ---

void ProcessingThread::process_and_insert_batch(const std::vector<TickerData>& batch) {
    if (batch.empty()) return;

    if (!db_manager_.begin_transaction()) {
        cerr << "FATAL: Could not start DB transaction." << endl;
        return;
    }

    int success_count = 0;
    for (const auto& data : batch) {
        //  Insert Raw Data
        if (db_manager_.insert_raw_data(data)) {
            success_count++;

            // Insert Aggregated Metrics (unchanged dummy calculation)
            double dummy_vwap = (data.high + data.low) / 2.0;
            double dummy_avg = (data.open + data.close) / 2.0;
            double calculated_ema_20 = calculate_ema(data.close, EMA_PERIOD_20);
            double calculated_ema_50 = calculate_ema(data.close, EMA_PERIOD_50);

            db_manager_.insert_metrics(
                data.timestamp_ms,
                data.trade_id, 
                data.symbol, 
                dummy_vwap, 
                dummy_avg,
                calculated_ema_20,
                calculated_ema_50
            );
        }
    }

    if (db_manager_.commit_transaction()) {
        cout << "[BATCH] Successfully committed " << success_count << " rows to DB." << endl;
    } else {
        cerr << "FATAL: Transaction commit failed. Rolling back." << endl;
        db_manager_.rollback_transaction();
    }
}


// --- Main Processing Logic (process_data_loop) ---

void ProcessingThread::process_data_loop() {
    std::vector<TickerData> current_batch;
    auto last_flush_time = std::chrono::steady_clock::now();
    
    while (running_) {
        std::optional<TickerData> data_opt = data_queue_.try_pop(); 

        if (data_opt.has_value()) {
            current_batch.push_back(data_opt.value());
            last_flush_time = std::chrono::steady_clock::now();

            if (current_batch.size() >= BATCH_SIZE) {
                process_and_insert_batch(current_batch); 
                current_batch.clear();
            }
        } else {
            if (!running_) {
                if (!current_batch.empty()) {
                    cout << "[SHUTDOWN FLUSH] Processing final batch of " << current_batch.size() << " items." << endl;
                    process_and_insert_batch(current_batch);
                }
                break; 
            }
        auto now = std::chrono::steady_clock::now();
            if (!current_batch.empty() && 
                std::chrono::duration_cast<std::chrono::milliseconds>(now - last_flush_time).count() > 500) { // 500 ms timeout
                
                std::cout << "[TIMEOUT FLUSH] Processing final batch of " << current_batch.size() << " items due to timeout." << std::endl;
                process_and_insert_batch(current_batch);
                current_batch.clear();
                last_flush_time = now;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        }
    }
}

double ProcessingThread::calculate_ema(double current_price, int period) {

    double& last_ema_value = (period == EMA_PERIOD_20) ? last_ema_value_20_ : last_ema_value_50_;
    bool& is_first_ema = (period == EMA_PERIOD_20) ? is_first_ema_20_ : is_first_ema_50_;

    const double multiplier = 2.0 / (static_cast<double>(period) + 1.0);
    
    double new_ema;
    
    if (is_first_ema) {
        new_ema = current_price;
        is_first_ema = false;
    } else {
        new_ema = (current_price * multiplier) + (last_ema_value * (1.0 - multiplier));
    }

    last_ema_value = new_ema;
    
    return new_ema;
}