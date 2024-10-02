#include "uCSerial/msgparser/msgparser.h"
#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

#include <iostream>
#include <unistd.h>

// Main loop
bool MessageParser::run() {

  // Run parser engine. Calls forceExit upon error.
  parserEngine.run();

  // Wait untill any key is pressed or forceExit writes to pipe.
  pipe(exitPipe.data());
  SerialUtils::waitForKeypress(exitPipe);

  // Stop parser.
  parserEngine.stop();
  std::cout << "5/6 Message parser finished." << std::endl;

  return true;
}

// Callback for parser engine.
bool MessageParser::forceExit(const std::string &errorMessage) const {
  // Show error message.
  std::cout << "Message parser exit: ";
  std::cout << errorMessage << std::endl;

  // Exit WaitForKeypress
  write(exitPipe[1], "x", 1);

  return true;
}
