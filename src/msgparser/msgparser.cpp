#include <iostream>
#include <unistd.h>

#include "uCSerial/msgparser/msgparser.h"
#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

// Main loop
bool MessageParser::Run() {

    // Run parser engine untill any key is pressed.
    parserEngine.Run([this](const std::string &error_message) { ForceExit(error_message); });

    pipe(exit_pipe.data());
    SerialUtils::WaitForKeypress(exit_pipe);

    // Stop parser.
    parserEngine.Stop();
    std::cout << "5/6 Message parser finished." << std::endl;

    return true;
}


// Callback for parser engine.
bool MessageParser::ForceExit(const std::string &error_message) const {
    // Show error message.
    std::cout << "Message parser exit: ";
    std::cout << error_message << std::endl;
    
    // Exit WaitForKeypress
    write(exit_pipe[1], "x", 1);

    return true;
}
