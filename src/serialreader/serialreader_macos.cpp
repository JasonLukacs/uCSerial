#include "uCSerial/serialreader/serialreader.h"

#include <jsonparser.h>

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int SerialReader::Run(
    const std::string &path, const std::function<void()> &onSerialDataAvailable,
    const std::function<void(std::string errorMessage)> &call_back_onError) {

  onError = call_back_onError;

  OpenSerialPort(path);
  StartReadingPort(onSerialDataAvailable);

  return serial_buffer_size;
}

bool SerialReader::StartReadingPort(
    const std::function<void()> &onSerialDataAvailable) {
  auto threadPtr = std::make_unique<std::thread>(
      [this, onSerialDataAvailable]() { ReadPort(onSerialDataAvailable); });
  portreadingThread = std::move(threadPtr);

  return true;
}

bool SerialReader::Stop() {
  if (portreadingThread) {
    // Write to the pipe to trigger poll()
    write(pipefd[1], "x", 1); // Write any data to trigger poll()
    portreadingThread->join();
    std::cout << "2/6 Thread finished: SerialReader." << std::endl;
    CloseSerialPort();
  }
  return true;
}

template <typename Callback>
  requires std::is_same_v<void, std::invoke_result_t<Callback>>
void SerialReader::ReadPort(Callback onSerialDataAvailable) {

  bool runReadPort = true;
  int pollReturnValue = -1;
  int ioctlReturnValue = 1;
  int bytesAvailable = 0;

  std::array<pollfd, 2> fds;

  fds[0].fd = serial_file_handle;
  fds[0].events = POLLIN;

  pipe(pipefd.data());
  fds[1].fd = pipefd[0];
  fds[1].events = POLLIN;

  while (runReadPort) {
    pollReturnValue = poll(fds.data(), 2, serial_timeout);
    ioctlReturnValue = ioctl(serial_file_handle, FIONREAD, &bytesAvailable);

    if (pollReturnValue > 0 && ioctlReturnValue >= 0 &&
        (fds[0].revents & POLLIN)) { // Happy flow.
      onSerialDataAvailable();
    } else if (fds[1].revents & POLLIN) { // Received quit signal on pipe.
      runReadPort = false;
    } else if (pollReturnValue == 0) { // Serial Timeout.
      onError("Serial timeout.");
    } else { // Something is seriously wrong.
      onError("Unknown serial error.");
      runReadPort = false;
    }
  }

  std::cout << "1/6 Thread function finished: SerialReader::ReadPort()"
            << std::endl;
}

int SerialReader::Read(std::vector<char> &buffer) const {

  ssize_t bytes_read =
      read(serial_file_handle, buffer.data(), serial_buffer_size);

  if (bytes_read == -1) {
    onError("Serial error.");
  }
  return int(bytes_read);
}

bool SerialReader::OpenSerialPort(const std::string &path) {
  // Get SerialConfig object
  SerialConfiguration serialConfig = LoadSerialConfiguration(path);

  serial_file_handle =
      open(serialConfig.serial_port.c_str(), O_RDWR | O_NOCTTY);
  if (serial_file_handle == -1) {
    
    //zoek deze case leidt niet tot program exit!!!
    std::string error_message = "Fatal error: Failed to open serial port ";
    
    error_message += serialConfig.serial_port.c_str();
    error_message += ".";
    throw SerialReaderException(error_message);
  }

  // Configure the serial port
  struct termios options;

  tcgetattr(serial_file_handle, &options);
  cfsetispeed(&options, serialConfig.baud_rate);
  cfsetospeed(&options, serialConfig.baud_rate);

  options.c_cflag &= ~CSIZE;
  options.c_cflag |=
      (serialConfig.data_bits - 5) *
      256; // 5, 6, 7, 8 bits: CS5, CS6, CS7, CS8:  0, 256, 512, 768

  if (serialConfig.parity) {
    options.c_cflag |= PARENB; //  Parity.
  } else {
    options.c_cflag &= ~PARENB; // No parity.
  }

  if (serialConfig.stop_bits == 2) {
    options.c_cflag |= CSTOPB; // 2 stop bits.
  } else {
    options.c_cflag &= ~CSTOPB; // 1 stop bit.
  }

  options.c_cflag |=
      CREAD | CLOCAL; // Enable receiver, ignore modem status lines.

  if (tcsetattr(serial_file_handle, TCSANOW, &options) == -1) {
    std::string error_message = "Failed to configure serial port, check "
                                "configuration file. Serial port: ";
    error_message += serialConfig.serial_port.c_str();
    throw SerialReaderException(error_message);
  }

  serial_buffer_size = serialConfig.buffer_size <= MAX_BUFFER
                           ? serialConfig.buffer_size
                           : MAX_BUFFER;
  serial_timeout = serialConfig.timeout;

  return true;
}

bool SerialReader::CloseSerialPort() const {
  close(serial_file_handle);
  std::cout << "3/6 Serial port closed." << std::endl;

  return true;
}

SerialConfiguration
SerialReader::LoadSerialConfiguration(const std::string &path) const {
  std::string serial_config_JSON_schema =
      path + "/schema/serial_config_schema.json";
  std::string serial_config_file = path + "/serialconfig.json";

  // Validate config file, schema, and config file against schema.
  try {
    JSONParser JSONparser;
    JSONparser.JSONValidate(serial_config_JSON_schema, serial_config_file);
  } catch (const JSONParserException &e) {
    throw SerialReaderException(e.what());
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
  config.buffer_size = document["buffer_size"].GetInt();
  config.timeout = document["timeout"].GetInt();

  return config;
}
