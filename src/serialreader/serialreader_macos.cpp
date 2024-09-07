#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <thread>

#include <jsonparser.h>

#include "uCSerial/serialreader/serialreader.h"


bool SerialReader::SetBufferSize(int buffer_size){
    serial_buffer_size = buffer_size <= MAX_BUFFER ? buffer_size : MAX_BUFFER;
    return true;
}


int SerialReader::GetBufferSize() const{
    return serial_buffer_size;
}


bool SerialReader::StartReadingPort(const std::function<void()> &callback) {
    OpenSerialPort();

    auto threadPtr = std::make_unique<std::thread>([this, callback]() {
        ReadPort(callback);
    });
    portreadingThread = std::move(threadPtr);

    return true;
}


bool SerialReader::StopReadingPort() {
    if (portreadingThread) {
        port_monitor_run = false;
        portreadingThread->join();
    }
    return true;
}


template<typename Callback>
bool SerialReader::ReadPort(Callback callback) const {
    // Set up select parameters
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serial_file_handle, &readfds);
    struct timeval tv;
    tv.tv_sec = 60;  // Timeout in seconds
    tv.tv_usec = 0;  // Timeout in microseconds

    while (port_monitor_run) {
        // Wait for data or timeout
        int ret = select(serial_file_handle + 1, &readfds, nullptr, nullptr, &tv);
        if (ret > 0) {  // ==0:timeout, ==-1:error
            // Data is available, call callback
            callback();
        }
    }
    return true;
}


int SerialReader::GetBytesAvailable() const {
    int bytesAvailable;
    ioctl(serial_file_handle, FIONREAD, &bytesAvailable);
    return bytesAvailable;
}


int SerialReader::ReadToBuffer(std::vector<char> &buffer) const {
    ssize_t bytes_read = read(serial_file_handle, buffer.data(), serial_buffer_size);

    if (bytes_read == -1) {
        // handle error
    }
    return int(bytes_read);
}


bool SerialReader::OpenSerialPort() {
    //! see Boost.Asio for portable solution.

    // Get SerialConfig object
    SerialConfiguration serialConfig = LoadSerialConfiguration();

    serial_file_handle = open(serialConfig.serial_port.c_str(), O_RDWR | O_NOCTTY);
    if (serial_file_handle == -1) {
        std::string error_message = "Failed to open serial port ";
        error_message += serialConfig.serial_port.c_str();
        throw SerialReaderConfigSerialPortException(error_message);
    }

    // Configure the serial port
    struct termios options;

    // Get attributes
    tcgetattr(serial_file_handle, &options);

    // Apply options from config object.
    cfsetispeed(&options, serialConfig.baud_rate);  // 115200 baud.
    cfsetospeed(&options, serialConfig.baud_rate);

    options.c_cflag &= ~CSIZE;
    options.c_cflag |= (serialConfig.data_bits - 5) * 256;  // 5, 6, 7, 8 bits: CS5, CS6, CS7, CS8:  0, 256, 512, 768

    if (serialConfig.parity) {
        options.c_cflag |= PARENB;  //  Parity.
    } else {
        options.c_cflag &= ~PARENB;  // No parity.
    }

    if (serialConfig.stop_bits == 2) {
        options.c_cflag |= CSTOPB;  // 2 stop bits.
    } else {
        options.c_cflag &= ~CSTOPB;  // 1 stop bit.
    }

    options.c_cflag |= CREAD | CLOCAL;  // Enable receiver, ignore modem status lines.

    // Set configuration.
    if (tcsetattr(serial_file_handle, TCSANOW, &options) == -1) {
        std::string error_message = "Failed to configure serial port, check configuration file. Serial port:  ";
        error_message += serialConfig.serial_port.c_str();
        throw SerialReaderConfigSerialPortException(error_message);
    }

    return true;
}


bool SerialReader::CloseSerialPort() const {
    close(serial_file_handle);
    return true;
}


SerialConfiguration SerialReader::LoadSerialConfiguration() const {
    std::string serial_config_JSON_schema = "../include/uCSerial/serialreader/serial_config_schema.json";
    std::string serial_config_file = "../config/serialconfig.json";

    // Validate config file, schema, and config file against schema.
    try {
        JSONParser JSONparser;
        JSONparser.JSONValidate(serial_config_JSON_schema, serial_config_file);
    } catch (const JSONParserException &e) {
        throw SerialReaderConfigSerialPortException(e.what());
    }

    // Validated, safe to create a rapidjson document.
    JSONParser JSONparser;
    rapidjson::Document document = JSONparser.GetJSONDocument(serial_config_file);

    // Populate return object
    SerialConfiguration config;
    config.serial_port = document["serial_port"].GetString();
    config.baud_rate = document["baud_rate"].GetInt();
    config.data_bits = document["data_bits"].GetInt();
    config.parity = document["parity"].GetBool();
    config.stop_bits = document["stop_bits"].GetInt();

    return config;
}
