#include <iostream>
#include <unistd.h>

#include "uCSerial/msgparser/msgparser.h"
#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

// Main loop
bool MessageParser::Run() {

    parserEngine.Run(path);

    // Run untill any key is pressed or pipe is written to.
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


bool MessageParser::ForceExit() const {
    // Write to the pipe monitored by uCSerialUtils::WaitForKeypress to trigger poll()
    write(pipefd[1], "x", 1);

    return true;
}
