#include <iostream>
#include <unistd.h>

#include "uCSerial/msgparser/msgparser.h"
#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

// Main loop
bool MessageParser::Run() {

    // Run parser engine untill any key is pressed.
    parserEngine.Run([this](const std::string &error_message) { Stop(error_message); });

    if (pipe(pipefd.data()) == -1) {
        std::string error_message = "Failed to create pipe. ";
        throw MsgParserException(error_message);
    }
    SerialUtils::WaitForKeypress(pipefd);

    // Stop parser.
    parserEngine.Stop();
    std::cout << "5/6 Message parser finished." << std::endl;

    return true;
}


bool MessageParser::Stop(const std::string &error_message) const {
    // Write to the pipe monitored by uCSerialUtils::WaitForKeypress to trigger poll()
    std::cout << error_message << std::endl;
    write(pipefd[1], "x", 1);

    return true;
}
