#include "../include/Persistence.h"
#include "../include/sqlite3.h" 
#include <iostream>

using namespace std;

PersistenceManager::PersistenceManager() : db_handle(nullptr) {}

PersistenceManager::~PersistenceManager() {
    close_db();
}


bool PersistenceManager::open_db() {

    int rc = sqlite3_open(DB_FILE.c_str(), (sqlite3**)&db_handle);

    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg((sqlite3*)db_handle) << endl;
        db_handle = nullptr;
        return false;
    } else {
        cout << "Database successfully opened: " << DB_FILE << endl;
        return true;
    }
}

void PersistenceManager::close_db() {
    if (db_handle) {
        sqlite3_close((sqlite3*)db_handle);
        db_handle = nullptr;
        cout << "Database closed." << endl;
    }
}

// --- Data Insertion Logic ---

bool PersistenceManager::insert_raw_data(const TickerData& data) {
    if (!db_handle) {
        cerr << "DB not open." << endl;
        return false;
    }

    const char* sql = "INSERT OR IGNORE INTO raw_ohlcv_data (open_time_ms, trade_id, symbol, open_price, high_price, low_price, close_price, volume) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    // Prepare the statement
    int rc = sqlite3_prepare_v2((sqlite3*)db_handle, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error on prepare: " << sqlite3_errmsg((sqlite3*)db_handle) << endl;
        return false;
    }

    // Bind parameters (Note: Indexing starts at 1)
    sqlite3_bind_int64(stmt, 1, data.timestamp_ms);
    sqlite3_bind_int64(stmt, 2, data.trade_id);
    sqlite3_bind_text(stmt, 3, data.symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, data.open);
    sqlite3_bind_double(stmt, 5, data.high);
    sqlite3_bind_double(stmt, 6, data.low);
    sqlite3_bind_double(stmt, 7, data.close);
    sqlite3_bind_double(stmt, 8, data.volume);

    // Execute the statement
    rc = sqlite3_step(stmt);
    
    // Finalize the statement (clean up)
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cerr << "Insertion failed: " << sqlite3_errmsg((sqlite3*)db_handle) << endl;
        return false;
    }
    
    // cout << "Raw data inserted successfully." << endl;
    return true;
}

bool PersistenceManager::insert_metrics(long long timestamp, long long trade_id, const std::string& symbol, double vwap, double simple_avg, double ema_20, double ema_50) {
    if (!db_handle) return false;

    // Prepared statement for aggregated metrics
    const char* sql = "INSERT OR IGNORE INTO aggregated_metrics (open_time_ms, trade_id, symbol, vwap, simple_average, ema_20, ema_50) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2((sqlite3*)db_handle, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Metrics prepare error: " << sqlite3_errmsg((sqlite3*)db_handle) << endl;
        return false;
    }

    // Bind parameters
    sqlite3_bind_int64(stmt, 1, timestamp);
    sqlite3_bind_int64(stmt, 2, trade_id);
    sqlite3_bind_text(stmt, 3, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, vwap);
    sqlite3_bind_double(stmt, 5, simple_avg);
    sqlite3_bind_double(stmt, 6, ema_20);
    sqlite3_bind_double(stmt, 7, ema_50);

    // Execute and finalize
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cerr << "Metrics insertion failed: " << sqlite3_errmsg((sqlite3*)db_handle) << endl;
        return false;
    }
    return true;
}

bool PersistenceManager::execute_sql(const char* sql) {
    if (!db_handle) return false;
    char* err_msg = 0;
    int rc = sqlite3_exec((sqlite3*)db_handle, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (" << sql << "): " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

// --- Kontrola Transakcija  ---
bool PersistenceManager::begin_transaction() {
    return execute_sql("BEGIN TRANSACTION;");
}

bool PersistenceManager::commit_transaction() {
    std::cout << "[DB] Committing transaction..." << std::endl;
    return execute_sql("COMMIT;");
}

bool PersistenceManager::rollback_transaction() {
    return execute_sql("ROLLBACK;");
}