#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <array>
#include <functional>
#include <thread>

#include "exceptions.h"
#include "serialconfig.h"

class SerialReader {
  public:
    enum class Result {
        SUCCESS = 0,
        TIMEOUT,
        ERROR,
    };

    bool InitSerial(const std::string &path);
    bool StartReadingPort(const std::function<void()> &onSerialDataAvailable, const std::function<void(std::string errorMessage)> &onError);
    bool StopReadingPort();
    int Read(std::vector<char> &buffer) const;
    int GetBufferSize() const;
    int GetBytesAvailable() const;
    
  private:
    int serial_buffer_size = 0;
    int serial_timeout = 0;
    int serial_file_handle = -1;
    std::unique_ptr<std::thread> portreadingThread;
    std::array<int, 2> pipefd;

    SerialConfiguration LoadSerialConfiguration(const std::string &path) const;
    bool OpenSerialPort(const std::string &path);
    bool CloseSerialPort() const;

    void ReadPort(const std::function<void()> &onSerialDataAvailable, const std::function<void(std::string errorMessage)> &onError);
};

#endif