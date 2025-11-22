-- 1. Table for Raw (OHLCV) Candlestick Data
CREATE TABLE IF NOT EXISTS raw_ohlcv_data (
    trade_id INTEGER NOT NULL,  
    symbol TEXT NOT NULL,
    open_time_ms INTEGER NOT NULL,
    
    open_price REAL NOT NULL,
    high_price REAL NOT NULL,
    low_price REAL NOT NULL,
    close_price REAL NOT NULL,
    volume REAL NOT NULL,
    
    ingestion_timestamp TEXT DEFAULT (strftime('%Y-%m-%d %H:%M:%S', 'now', 'localtime')),
    
    PRIMARY KEY (trade_id, symbol) 
);

CREATE INDEX IF NOT EXISTS idx_raw_symbol ON raw_ohlcv_data (symbol);
CREATE INDEX IF NOT EXISTS idx_raw_time ON raw_ohlcv_data (open_time_ms);


-- 2. Table for Aggregated/Processed Metrics
CREATE TABLE IF NOT EXISTS aggregated_metrics (
    trade_id INTEGER NOT NULL,
    symbol TEXT NOT NULL,
    open_time_ms INTEGER NOT NULL,
    
    -- Metrike
    vwap REAL, 
    simple_average REAL,
    ema_20 REAL,
    ema_50 REAL,
    
    PRIMARY KEY (trade_id, symbol) 
);

CREATE INDEX IF NOT EXISTS idx_metrics_symbol ON aggregated_metrics (symbol);
CREATE INDEX IF NOT EXISTS idx_metrics_time ON aggregated_metrics (open_time_ms);