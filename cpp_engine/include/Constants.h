#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// --- Socket Communication Parameters ---
const std::string SERVER_IP = "127.0.0.1"; // Localhost IP
const int SERVER_PORT = 12345;           // Port for Python client to connect to

// --- Application Settings ---
const int MAX_CLIENTS = 5;     
const int BATCH_SIZE = 100;    
const int TIMEOUT_MS = 5000;     

#endif 