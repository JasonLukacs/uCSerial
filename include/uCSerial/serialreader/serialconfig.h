#ifndef SERIALCONFIG_H
#define SERIALCONFIG_H

#include <string>

const int MAX_BUFFER = 1024;

struct SerialConfiguration {
    std::string serial_port;
    int baud_rate;
    int data_bits;
    bool parity;
    int stop_bits;
    int buffer_size;
    int timeout;
};

#endif