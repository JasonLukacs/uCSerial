#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <array>
#include <functional>
#include <thread>

#include "exceptions.h"
#include "serialconfig.h"

class SerialReader {
   public:
    SerialReader() = default;

    enum class ReadResult {
        SUCCESS = 0,
        ERROR_TIMEOUT,
        ERROR_INVALID_DATA,
        // ... other possible error codes
    };

    bool SetBufferSize(int buffer_size);
    int GetBufferSize() const;

    bool StartReadingPort(const std::function<void(int)> &callback);
    bool StopReadingPort();

    int GetBytesAvailable() const;
    int ReadToBuffer(std::vector<char> &buffer) const;

    ~SerialReader() { CloseSerialPort(); }

   private:
    int serial_buffer_size = 1024;
    int serial_timeout = 60000;
    int serial_file_handle = -1;
    std::unique_ptr<std::thread> portreadingThread;
    std::array<int, 2> pipefd;

    SerialConfiguration LoadSerialConfiguration() const;
    bool OpenSerialPort();
    bool CloseSerialPort() const;

    template <typename Callback>
    bool ReadPort(Callback callBack);
};

#endif