#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/msgparser/msgparser.h"


bool MessageParser::Start() {
    // Create a serial reader and set buffer size.
    this->serialReader = std::make_unique<SerialReader>();
    serialReader->SetBufferSize(256);

    // Start monitoring the serial port
    try {
        serialReader->StartReadingPort([this]() {
            ReadData();
        });
    } catch (const SerialReaderException& e) {
        throw MsgParserException(e.what());
    }

    return true;
}


bool MessageParser::Stop() {
    // Stop reading serial port.
    serialReader->StopReadingPort();
    return true;
}


bool MessageParser::ReadData() const {
    // Callback function for SerialReader.
    // Data available. Dump to terminal.

    std::vector<char> buffer(serialReader->GetBufferSize());
    int bytes_read = serialReader->ReadToBuffer(buffer);

    std::cout << "Bytes read:      " << bytes_read << std::endl;
    std::cout << "Data received:" << std::endl;
    for (int i = 0; i < bytes_read; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;

    return true;
}
