#include "../include/DataIngestor.h"
#include "../include/TickerData.h"
#include "../include/Constants.h"
#include <sstream>
#include <string.h>
#include <iostream>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

using namespace std;

DataIngestor::DataIngestor(SafeQueue<TickerData>& queue)
    : data_queue_(queue), server_socket_(-1), running_(false) {
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
}

DataIngestor::~DataIngestor() {
    stop_server();
    #ifdef _WIN32
        WSACleanup();
    #endif
}

// --- Server Startup ---

void DataIngestor::start_server() {

    running_ = true; 

    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        cerr << "FATAL: Could not create server socket." << endl;
        running_ = false;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);

    if (::bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "FATAL: Bind failed. Port " << SERVER_PORT << " may be in use." << endl;
        running_ = false;
        return;
    }

    if (listen(server_socket_, MAX_CLIENTS) < 0) {
        cerr << "FATAL: Listen failed." << endl;
        running_ = false;
        return;
    }

    cout << "TCP Server started on " << SERVER_IP << ":" << SERVER_PORT << ". Waiting for client..." << endl;

    while (running_) {
        sockaddr_in client_addr;
        #ifdef _WIN32
            int client_len = sizeof(client_addr);
        #else
            socklen_t client_len = sizeof(client_addr);
        #endif

        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            if (!running_) {
                break;
            }
            cerr << "Error accepting connection." << endl;
            continue;
        }

        cout << "Client connected. Starting thread..." << endl;
        client_threads_.emplace_back(&DataIngestor::handle_client, this, client_socket);
    }
}

// --- Client Handler (Receives and Processes Data) ---

void DataIngestor::handle_client(int client_socket) {
    char buffer[4096];
    int bytes_received;

    while (running_ && (bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; 

        stringstream ss(buffer);
        string line;

        while (getline(ss, line, '\n')) {
            if (line.empty()) continue;

            try {
                TickerData data = parseTickerData(line);
                data_queue_.push(data);
            } catch (const exception& e) {
                cerr << "Parsing error: " << e.what() << " | Data: " << line << endl;
            }
        }
    }

    #ifdef _WIN32
        closesocket(client_socket);
    #else
        close(client_socket);
    #endif

    cout << "Client disconnected." << endl;
}

// --- Shutdown ---

void DataIngestor::stop_server() {
    if (!running_) return;

    running_ = false;

    if (server_socket_ > 0) {
        #ifdef _WIN32
            closesocket(server_socket_);
        #else
            close(server_socket_);
        #endif
    }

    for (auto& t : client_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    client_threads_.clear();
    cout << "Server stopped and threads joined." << endl;
}