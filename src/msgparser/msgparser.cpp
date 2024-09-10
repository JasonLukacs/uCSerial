#include "uCSerial/msgparser/msgparser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "uCSerial/serialreader/serialreader.h"

bool MessageParser::Start() {
    // Create a serial reader and set buffer size.
    this->serialReader = std::make_unique<SerialReader>();
    serialReader->SetBufferSize(256);

    // Start monitoring the serial port
    try {
        serialReader->StartReadingPort([this](int error) { ReadData(error); });
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

bool MessageParser::ReadData(int error) const {
    // Callback function for SerialReader.
    // Data available. Dump to terminal.
    if (!error) {
        std::vector<char> buffer(serialReader->GetBufferSize());
        int bytes_read = serialReader->ReadToBuffer(buffer);

        std::cout << "Bytes read:      " << bytes_read << std::endl;
        std::cout << "Data received:" << std::endl;
        for (int i = 0; i < bytes_read; ++i) {
            std::cout << buffer[i];
        }
        std::cout << std::endl;
    } else if (error == 1) {
        std::cout << "Serial timed out." << std::endl;
        exit(1);
    } else {
        std::cout << "Serial error." << std::endl;
        exit(1);
    }

    return true;
}
