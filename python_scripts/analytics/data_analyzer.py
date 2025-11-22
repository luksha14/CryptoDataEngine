import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import os
import time

DB_PATH = '../../db_setup/crypto_data.db' 

def get_db_path():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    relative_path = os.path.join(current_dir, DB_PATH)
    db_full_path = os.path.abspath(relative_path)
    return db_full_path

def load_data(symbol, table_name):
    """ Loads data for a given table. """
    db_file_path = get_db_path()
    conn = None
    df = pd.DataFrame()
    
    try:
        conn = sqlite3.connect(db_file_path)
        print(f"Connected to database: {db_file_path}")
        
        if table_name == 'aggregated_metrics':
             query = f"""
                SELECT 
                    open_time_ms, 
                    trade_id,
                    symbol, 
                    vwap, 
                    ema_20,
                    ema_50  
                FROM aggregated_metrics
                WHERE symbol = '{symbol}'
                ORDER BY open_time_ms ASC
            """
        elif table_name == 'raw_ohlcv_data':
            query = f"""
                SELECT 
                    open_time_ms, 
                    trade_id,
                    volume 
                FROM raw_ohlcv_data
                WHERE symbol = '{symbol}'
                ORDER BY open_time_ms ASC
            """
        else:
            return df

        df = pd.read_sql_query(query, conn)
        
    except sqlite3.Error as e:
        print(f"SQLite error while reading from {table_name}: {e}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if conn:
            conn.close()
            
    return df

def analyze_and_plot(symbol):
    
    df_metrics = load_data(symbol, 'aggregated_metrics')
    
    df_raw = load_data(symbol, 'raw_ohlcv_data')

    if df_metrics.empty or df_raw.empty:
        print(f"No data found for symbol {symbol} in the database.")
        return
    
    df = pd.merge(df_metrics, df_raw, on=['open_time_ms', 'trade_id'], how='inner')
    
    df['timestamp'] = pd.to_datetime(df['open_time_ms'], unit='ms')
    df.set_index('timestamp', inplace=True)
    
    # Logika dual EMA crossover signala i buy signala
    
    df['Crossover'] = df['ema_20'] - df['ema_50']
    
    df['Signal'] = 0.0
    # Postavi signal na 1.0 kada je EMA 20 iznad EMA 50
    df.loc[df['Crossover'] > 0, 'Signal'] = 1.0
    
    # 'Position' pronalazi trenutak kada se Signal promijenio s 0 na 1 (Buy)
    df['Position'] = df['Signal'].diff() 

    buy_signals = df[df['Position'] == 1.0]
    
    print(f"Loaded {len(df)} rows for {symbol}. Found {len(buy_signals)} Buy Signals (EMA 20 > EMA 50).")
    
    # --- Prikaz (Dva grafa) ---
    fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)
    fig.suptitle(f'Data Analysis for {symbol} (Dual EMA Crossover & Volume)', fontsize=16)

    # Price Chart (Gornji graf)
    axes[0].plot(df['vwap'], label='VWAP Price', color='blue')
    axes[0].plot(df['ema_20'], label='EMA 20 (Fast)', color='red', linestyle='--')
    axes[0].plot(df['ema_50'], label='EMA 50 (Slow)', color='purple', linestyle=':') # <--- NOVI EMA 50
    
    # Crtanje Buy Signala
    axes[0].plot(buy_signals.index, 
                 buy_signals['ema_20'], 
                 '^', markersize=10, color='green', label='Buy Signal (EMA 20 > 50)')
             
    axes[0].set_ylabel('Price ($)', fontsize=12)
    axes[0].grid(True)
    axes[0].legend(loc='upper left')

    # Volume Chart (Donji graf)
    axes[1].bar(df.index, df['volume'], label='Volume', color='green', width=0.0001)
    axes[1].set_xlabel('Time', fontsize=12)
    axes[1].set_ylabel('Volume', fontsize=12)
    axes[1].grid(True)
    axes[1].legend(loc='upper left')

    plt.xticks(rotation=45)
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

if __name__ == "__main__":
    # The symbol currently sent by the data fetcher
    symbol_to_analyze = "BTCUSDT" 
    
    print("--- Starting Data Analyzer ---")
    print("DB FULL PATH:", get_db_path())
    analyze_and_plot(symbol_to_analyze)
    print("--- Analysis finished. ---")