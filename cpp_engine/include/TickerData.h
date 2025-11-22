// File: /cpp_engine/include/TickerData.h

#ifndef TICKER_DATA_H
#define TICKER_DATA_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>

// Structure to hold one candlestick (OHLCV) data point
struct TickerData {
    long long timestamp_ms; 
    std::string symbol;     // e.g., "BTCUSDT"
    long long trade_id;
    double open;
    double high;
    double low;
    double close;
    double volume;

    /**
     * @brief Utility function to print the data struct. 
     * Defined inline because it's in a header file.
     */
    inline void print() const {
        std::time_t t = timestamp_ms / 1000;
        char buffer[30];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
        
        std::cout << "Time: " << buffer
                  << " | Symbol: " << symbol
                  << " | Close: " << close
                  << " | Volume: " << volume
                  << std::endl;
    }
};

TickerData parseTickerData(const std::string& csv_line);

#endif // TICKER_DATA_H