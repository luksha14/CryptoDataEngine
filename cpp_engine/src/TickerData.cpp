#include "../include/TickerData.h"
#include <sstream>
#include <vector>
#include <stdexcept> 
#include <string> 

TickerData parseTickerData(const std::string& csv_line) {
    TickerData data;
    std::stringstream ss(csv_line);
    std::string segment;
    std::vector<std::string> seglist;

    std::string cleaned_line = csv_line;
    if (!cleaned_line.empty() && cleaned_line.back() == '\r') {
        cleaned_line.pop_back();
    }
    
    std::stringstream clean_ss(cleaned_line); 

    while(std::getline(clean_ss, segment, ',')) {
        seglist.push_back(segment);
    }
    
    if (seglist.size() != 8) { 
        throw std::runtime_error("Invalid data format received. Expected 8 segments, got " + std::to_string(seglist.size()));
    }
    
    // Format seglist: [0]timestamp_ms, [1]symbol, [2]trade_id, [3]open, [4]high, [5]low, [6]close, [7]volume

    try {
        data.timestamp_ms = std::stoll(seglist[0]);
        data.symbol = seglist[1];
        data.trade_id = std::stoll(seglist[2]); 
        data.open = std::stod(seglist[3]);
        data.high = std::stod(seglist[4]);
        data.low = std::stod(seglist[5]);
        data.close = std::stod(seglist[6]);
        data.volume = std::stod(seglist[7]); 
    } catch (const std::exception& e) {
        throw std::runtime_error("Data conversion error during parsing: " + std::string(e.what()));
    }

    return data;
}