#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <array>
#include <functional>
#include <thread>

#include "exceptions.h"
#include "serialconfig.h"

class SerialReader {
   public:
     explicit SerialReader(const std::string& path) : path(path) {}

    enum class ReadResult {
        READ_SUCCESS = 0,
        READ_TIMEOUT,
        READ_ERROR,
    };

    int GetBufferSize() const;
    bool InitSerial();
    bool StartReadingPort(const std::function<void(SerialReader::ReadResult)> &callback);
    bool StopReadingPort();
    int GetBytesAvailable() const;
    int ReadToBuffer(std::vector<char> &buffer) const;

   private:
    std::string path;
    int serial_buffer_size = 0;
    int serial_timeout = 0;
    int serial_file_handle = -1;
    std::unique_ptr<std::thread> portreadingThread;
    std::array<int, 2> pipefd;

    SerialConfiguration LoadSerialConfiguration() const;
    bool OpenSerialPort();
    bool CloseSerialPort() const;

    template <typename Callback>
    requires std::is_invocable_v<Callback, ReadResult>
    void ReadPort(Callback callBack);
};

#endif