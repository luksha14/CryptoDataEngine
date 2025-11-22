
# 🚀 Crypto Data Engine (C++/Python)

This project features a high-performance core for real-time cryptocurrency data processing, ingestion, and storage. It consists of an efficient C++ Engine for data handling and persistence into an SQLite database, complemented by a Python analytics layer for visualization and signal generation.

## ✨ Key Features
* **High Performance:** Utilizes a multi-threaded architecture with a SafeQueue for asynchronous data ingestion and high-frequency batch processing.
* **Live Trade Stream Ingestion:** Data is received via a TCP socket, connected to a Binance WebSocket Trade Stream for real-time transaction ticks.
* **Data Integrity:** Implements the Trade ID as a unique primary key to ensure data integrity and prevent data loss during rapid ingestion.
* **Metric Calculation:** Real-time calculation and persistence of financial metrics, including VWAP, Simple Average, and the Exponential Moving Averages (EMA 20 & EMA 50).
* **Robustness:** Implements a Graceful Shutdown mechanism and a Timeout Flush to ensure no data is lost upon client disconnection or engine termination.
* **Signal Generation:** The Python analyzer identifies potential buy signals based on the EMA 20/50 Crossover and VWAP strategy.

## ⚙️ Technical Architecture Overview
The system is split into two primary components:

### 1. C++ Engine (Server)
The engine is built on multi-threading to handle I/O and processing concurrently:

* **DataIngestor:** Manages the TCP server, accepts client connections, and pushes high-frequency raw data ticks (including Trade ID) into the SafeQueue.
* **ProcessingThread:** A dedicated worker thread that asynchronously pops data from the SafeQueue in optimized batches (e.g., 40+ rows). It calculates VWAP, EMA 20, EMA 50, and commits batches to the database.
* **PersistenceManager:** Handles SQLite operations, prepared statements, and ensures transactional integrity using Trade ID as a unique constraint.

### 2. Python Tools (Client & Analysis)
* **binance_data_fetcher.py:** Connects to Binance WebSocket Trade Stream, formats ticks, and sends them to the C++ Engine over TCP.
* **data_analyzer.py:** Pulls VWAP, EMA20/EMA50, and volume data, performs crossover analysis, and visualizes signals.

## 🚀 Quick Start Guide

### 1. Project Initialization
```bash
git clone https://github.com/luksha14/CryptoDataEngine
cd CryptoDataEngine
```

### 2. Create and Activate Environment & Initialize Database
```bash
python -m venv venv
.env\Scriptsctivate
pip install -r requirements.txt

# Initialize the database
sqlite3 db_setup/crypto_data.db ".read db_setup/schema.sql"
```

### 3. Compile the C++ Engine
```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### 4. Run the Engine & Fetcher (In Separate Terminals)

#### Terminal 1 – Run the C++ Data Engine
```bash
cd build
.\data_engine.exe
```

#### Terminal 2 – Run the Live Data Fetcher (Client)
```bash
cd ..
.env\Scripts\python.exe python_scripts\feed_client\binance_data_fetcher.py
```

### 5. Run the Analysis
```bash
.env\Scripts\python.exe python_scripts\analytics\data_analyzer.py
```

## 👤 Author
Luka Mikulić
