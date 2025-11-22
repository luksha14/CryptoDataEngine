#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include "../include/Persistence.h" 
#include "../include/SafeQueue.h"
#include "../include/DataIngestor.h"
#include "../include/ProcessingThread.h"

using namespace std;

std::atomic<bool> g_running{true};

SafeQueue<TickerData>* g_queue = nullptr;
DataIngestor* g_ingestor = nullptr;
ProcessingThread* g_processor = nullptr;
PersistenceManager* g_dbManager = nullptr;

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        cout << "\n[SHUTDOWN] Signal (" << signum << ") received. Shutting down engine..." << endl;
        
        g_running = false; 
        
        if (g_ingestor) {
            g_ingestor->stop_server(); 
        }

    }
}

int main() {
    cout << "--- Crypto Data Engine Started ---" << endl;
    
    signal(SIGINT, signal_handler);
    
    PersistenceManager dbManager;
    g_dbManager = &dbManager;

    if (!dbManager.open_db()) {
        cerr << "FATAL: Could not connect to database. Exiting." << endl;
        return 1;
    }
    
    SafeQueue<TickerData> dataQueue;
    g_queue = &dataQueue; 
    
    ProcessingThread dataProcessor(dataQueue, dbManager);
    g_processor = &dataProcessor;
    dataProcessor.start_thread();
    
    DataIngestor dataIngestor(dataQueue);
    g_ingestor = &dataIngestor;

    dataIngestor.start_server(); 

    cout << "Main thread entering monitoring loop. Press CTRL+C to stop." << endl;
    
    while (g_running) {
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    
    cout << "Starting shutdown..." << endl;
    
    if (g_processor) {
        g_processor->stop_thread(); 
    }
    
    if (g_dbManager) {
        g_dbManager->close_db();
    }
    
    cout << "--- Engine Shutdown Complete ---" << endl;
    return 0;
}