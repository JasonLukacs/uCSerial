#include "uCSerial/msgparser/msgparser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

// Main loop
bool MessageParser::Run() {

    // Start monitoring the serial port
    this->serialReader = std::make_unique<SerialReader>(path);

    try {
        serialReader->InitSerial();
        buffer_size = serialReader->GetBufferSize();
        serialReader->StartReadingPort([this](SerialReader::ReadResult result) { ReadData(result); });

    } catch (const SerialReaderException &e) {
        throw MsgParserException(e.what());
    }

    // Run untill any key is pressed or pipe is written to.
    if (pipe(pipefd.data()) == -1) {
        std::string error_message = "Failed to create pipe. ";
        throw MsgParserException(error_message);
    }
    SerialUtils::WaitForKeypress(pipefd);

    // Stop parser.
    Stop();

    return true;
}

bool MessageParser::Stop() const {
    // Stop reading serial port.
    serialReader->StopReadingPort();
    std::cout << "4/5 MessageParser finished." << std::endl;
    return true;
}

bool MessageParser::ReadData(SerialReader::ReadResult result) const {
    // Callback function for SerialReader.
    using enum SerialReader::ReadResult;

    switch (result) {
    case READ_SUCCESS:
        PrintResult();
        break;
    case READ_TIMEOUT:
        std::cout << "Serial timed out." << std::endl;
        ForceExit();
        break;
    default:
        std::cout << "Serial error." << std::endl;
        ForceExit();
        break;
    }

    return true;
}

void MessageParser::PrintResult() const {
    std::vector<char> buffer(buffer_size);
    int bytes_read = serialReader->ReadToBuffer(buffer);

    std::cout << "Bytes read:      " << bytes_read << std::endl;
    std::cout << "Data received:" << std::endl;
    for (int i = 0; i < bytes_read; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
}

bool MessageParser::ForceExit() const {
    // Write to the pipe monitored by uCSerialUtils::WaitForKeypress to trigger poll()
    write(pipefd[1], "x", 1);

    return true;
}