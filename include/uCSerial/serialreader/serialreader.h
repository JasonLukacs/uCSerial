#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <thread>
#include <functional>
#include <array>

#include "exceptions.h"

const int MAX_BUFFER = 1024;

class SerialConfiguration {
  public:
    std::string serial_port = "/dev/cu.usbserial-110";
    int baud_rate = 115200;  // 115200 baud.
    int data_bits = 8;       // 8 data bits. Or 7, 6, 5.
    bool parity = false;     // No parity. Can be true.
    int stop_bits = 1;       // 1 stop bit. Can be 2.
};


class SerialReader {
  public:
    SerialReader() = default;

    bool SetBufferSize(int buffer_size);
    int GetBufferSize() const;

    bool StartReadingPort(const std::function<void()> &callback);
    bool StopReadingPort();

    int GetBytesAvailable() const;
    int ReadToBuffer(std::vector<char> &buffer) const;

    ~SerialReader() {
        CloseSerialPort();
    }

  private:
    int serial_buffer_size = 1024;
    std::atomic<bool> port_monitor_run{ true };
    int serial_file_handle = -1;
    std::unique_ptr<std::thread> portreadingThread;
    std::array<int, 2> pipefd;


    SerialConfiguration LoadSerialConfiguration() const;
    bool OpenSerialPort();
    bool CloseSerialPort() const;

    template<typename Callback>
    bool ReadPort(Callback callback) ;
};

#endif