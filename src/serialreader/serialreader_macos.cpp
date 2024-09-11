#include <fcntl.h>
#include <jsonparser.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <thread>

#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/serialreader/serialconfig.h"

bool SerialReader::SetBufferSize(int buffer_size) {
    serial_buffer_size = buffer_size <= MAX_BUFFER ? buffer_size : MAX_BUFFER;
    return true;
}

int SerialReader::GetBufferSize() const { return serial_buffer_size; }

bool SerialReader::StartReadingPort(const std::function<void(SerialReader::ReadResult)> &callback) {

    OpenSerialPort();

    auto threadPtr = std::make_unique<std::thread>(
        [this, callback]() { ReadPort(callback); });
    portreadingThread = std::move(threadPtr);

    return true;
}

bool SerialReader::StopReadingPort() {
    if (portreadingThread) {
        // Write to the pipe to trigger poll()
        write(pipefd[1], "y", 1);  // Write any data to trigger poll()
        portreadingThread->join();
    }
    return true;
}


template <typename Callback>
//bool SerialReader::ReadPort(std::function<bool(ReadResult)> callBack) {
bool SerialReader::ReadPort(Callback callBack) {
    if (pipe(pipefd.data()) == -1) {
        std::string error_message = "Failed to create pipe. ";
        throw SerialReaderException(error_message);
    }

    while (true) {
        std::array<pollfd, 2> fds;
        fds[0].fd = serial_file_handle;
        fds[0].events = POLLIN;
        fds[1].fd = pipefd[0];
        fds[1].events = POLLIN;

        int ret = poll(fds.data(), 2, serial_timeout);
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                callBack(ReadResult::READ_SUCCESS);
            } else if (fds[1].revents & POLLIN) {
                // Received quit signal on pipe.
                break;
            } else {
                callBack(ReadResult::READ_ERROR);
            }
        } else if (ret == 0) {
            callBack(ReadResult::READ_TIMEOUT);
        } else {
            callBack(ReadResult::READ_ERROR);
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
    ssize_t bytes_read =
        read(serial_file_handle, buffer.data(), serial_buffer_size);

    if (bytes_read == -1) {
        // handle error
    }
    return int(bytes_read);
}

bool SerialReader::OpenSerialPort() {
    // Get SerialConfig object
    SerialConfiguration serialConfig = LoadSerialConfiguration();

    serial_file_handle =
        open(serialConfig.serial_port.c_str(), O_RDWR | O_NOCTTY);
    if (serial_file_handle == -1) {
        std::string error_message = "Failed to open serial port ";
        error_message += serialConfig.serial_port.c_str();
        throw SerialReaderException(error_message);
    }

    // Configure the serial port
    struct termios options;

    // Get attributes
    tcgetattr(serial_file_handle, &options);

    // Apply options from config object.
    cfsetispeed(&options, serialConfig.baud_rate);  // 115200 baud.
    cfsetospeed(&options, serialConfig.baud_rate);

    options.c_cflag &= ~CSIZE;
    options.c_cflag |=
        (serialConfig.data_bits - 5) *
        256;  // 5, 6, 7, 8 bits: CS5, CS6, CS7, CS8:  0, 256, 512, 768

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

    options.c_cflag |=
        CREAD | CLOCAL;  // Enable receiver, ignore modem status lines.

    // Set configuration.
    if (tcsetattr(serial_file_handle, TCSANOW, &options) == -1) {
        std::string error_message =
            "Failed to configure serial port, check configuration file. Serial "
            "port:  ";
        error_message += serialConfig.serial_port.c_str();
        throw SerialReaderException(error_message);
    }

    // Config not bound to port.
    serial_buffer_size = serialConfig.buffer_size <= MAX_BUFFER ? serial_buffer_size : MAX_BUFFER;
    serial_timeout = serialConfig.timeout;

    return true;
}

bool SerialReader::CloseSerialPort() const {
    close(serial_file_handle);
    return true;
}

SerialConfiguration SerialReader::LoadSerialConfiguration() const {
    std::string serial_config_JSON_schema =
        "../include/uCSerial/serialreader/serial_config_schema.json";
    std::string serial_config_file = "../config/serialconfig.json";

    // Validate config file, schema, and config file against schema.
    try {
        JSONParser JSONparser;
        JSONparser.JSONValidate(serial_config_JSON_schema, serial_config_file);
    } catch (const JSONParserException &e) {
        throw SerialReaderException(e.what());
    }

    // Validated, safe to create a rapidjson document.
    JSONParser JSONparser;
    rapidjson::Document document =
        JSONparser.GetJSONDocument(serial_config_file);

    // Populate return object
    SerialConfiguration config;
    config.serial_port = document["serial_port"].GetString();
    config.baud_rate = document["baud_rate"].GetInt();
    config.data_bits = document["data_bits"].GetInt();
    config.parity = document["parity"].GetBool();
    config.stop_bits = document["stop_bits"].GetInt();
    config.buffer_size = document["max_buffer"].GetInt();
    config.timeout = document["timeout"].GetInt();

    return config;
}
