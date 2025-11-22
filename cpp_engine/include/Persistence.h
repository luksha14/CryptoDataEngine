#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include "TickerData.h"

const std::string DB_FILE = "../db_setup/crypto_data.db";

class PersistenceManager {
private:
    void* db_handle; 
    bool execute_sql(const char* sql);

public:
    PersistenceManager();
    ~PersistenceManager();

    bool open_db();
    void close_db();

    bool insert_raw_data(const TickerData& data);
    bool insert_metrics(long long timestamp, long long trade_id, const std::string& symbol, double vwap, double simple_avg, double ema_20, double ema_50);

    bool begin_transaction();
    bool commit_transaction();
    bool rollback_transaction();
};

#endif // PERSISTENCE_H