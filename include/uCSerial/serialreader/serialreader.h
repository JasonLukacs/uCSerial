#ifndef SERIALREADER_H
#define SERIALREADER_H

#include "serialconfig.h"

#include <array>
#include <thread>

class SerialReader {
public:
  int Run(
      const std::string &path,
      const std::function<void()> &onSerialDataAvailable,
      const std::function<void(std::string errorMessage)> &call_back_onError);
  int Read(std::vector<char> &buffer) const;
  bool Stop();

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

  template <typename Callback>
    requires std::is_same_v<void, std::invoke_result_t<Callback>>
  void ReadPort(Callback onSerialDataAvailable);
};

#endif