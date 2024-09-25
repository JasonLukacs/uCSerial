#include <iostream>
#include <unistd.h>

#include "uCSerial/msgparser/msgparser.h"
#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

// Main loop
bool MessageParser::Run() {

    // Run parser engine untill any key is pressed.
    parserEngine.Run([this](const std::string &error_message) { ForceExit(error_message); });

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


// Callback for parser engine in case of trouble.
bool MessageParser::ForceExit(const std::string &error_message) const {
    // Show error message.
    std::cout << "Message parser exit: ";
    std::cout << error_message << std::endl;
    
    // Exit WaitForKeypress
    write(pipefd[1], "x", 1);

    return true;
}
