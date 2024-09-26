#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <array>
#include <functional>
#include <thread>

#include "exceptions.h"
#include "serialconfig.h"

class SerialReader {
  public:
    int Run(
        const std::string &path,
        const std::function<void()> &onSerialDataAvailable,
        const std::function<void(std::string errorMessage)> &call_back_onError
    );
    int Read(std::vector<char> &buffer) const;
    bool Stop();
    int GetBufferSize() const;
    int GetBytesAvailable() const;

  private:
    int serial_buffer_size = 0;
    int serial_timeout = 0;
    int serial_file_handle = -1;
    std::function<void(std::string)> onError;
    std::unique_ptr<std::thread> portreadingThread;
    std::array<int, 2> pipefd;

    SerialConfiguration LoadSerialConfiguration(const std::string &path) const;
    bool OpenSerialPort(const std::string &path);
    bool StartReadingPort(const std::function<void()> &onSerialDataAvailable);
    bool CloseSerialPort() const;

    void ReadPort(const std::function<void()> &onSerialDataAvailable);
};

#endif