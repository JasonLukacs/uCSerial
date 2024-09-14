#include "uCSerial/msgparser/msgparser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

bool MessageParser::Run() {
    // Start monitoring the serial port

    try {
        this->serialReader = std::make_unique<SerialReader>();
        serialReader->StartReadingPort(
            [this](SerialReader::ReadResult result) { ReadData(result); });
    } catch (const SerialReaderException& e) {
        throw MsgParserException(e.what());
    }

    // Fetch buffer size. Set in serialconfig.json.
    buffer_size = serialReader->GetBufferSize();

    // Run untill any key is pressed.
    uCSerialUtils::WaitForKeypress();

    // Stop parser.
    Stop();

    return true;
}

bool MessageParser::Stop() {
    // Stop reading serial port.
    serialReader->StopReadingPort();
    std::cout << "4/5 MessageParser finished." << std::endl;
    return true;
}

bool MessageParser::ReadData(SerialReader::ReadResult result) const {
    // Callback function for SerialReader.
    switch (result) {
        case SerialReader::ReadResult::READ_SUCCESS:
            PrintResult();
            break;
        case SerialReader::ReadResult::READ_TIMEOUT:
            std::cout << "Serial timed out." << std::endl;
            // zoek
            exit(1);
        default:
            std::cout << "Serial error." << std::endl;
            // zoek
            exit(1);
    }

    return true;
}

void MessageParser::PrintResult() const {
    std::vector<char> buffer(buffer_size);
    int bytes_read = serialReader->ReadToBuffer(buffer);

    std::cout << "Buffer size:     " << serialReader->GetBufferSize()
              << std::endl;
    std::cout << "Bytes read:      " << bytes_read << std::endl;
    std::cout << "Data received:" << std::endl;
    for (int i = 0; i < bytes_read; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
}