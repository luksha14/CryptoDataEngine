// File: /cpp_engine/include/DataIngestor.h

#ifndef DATA_INGESTOR_H
#define DATA_INGESTOR_H

#include <thread>
#include <vector>
#include <iostream>
#include <functional>
#include "SafeQueue.h"
#include "Constants.h"

/**
 * @brief Manages the TCP/IP server responsible for receiving data from Python clients.
 */
class DataIngestor {
private:
    SafeQueue<TickerData>& data_queue_;
    std::vector<std::thread> client_threads_;
    bool running_ = true;
    int server_socket_;

    // Handle data reception from a single client
    void handle_client(int client_socket);

public:
    DataIngestor(SafeQueue<TickerData>& queue);
    ~DataIngestor();

    // Starts the main server thread
    void start_server();

    // Stops the server and joins all threads
    void stop_server();
};

#endif // DATA_INGESTOR_H