/*
class               PascalCase
object/pointer      camelCase
variable            snake_case
function            PascalCase

*/

#include <iostream>

#include "uCSerial/msgparser/msgparser.h"

int main() {
  // Run the message parser.
  MessageParser messageParser;
  try {
    messageParser.Run();
  } catch (const MsgParserException &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // Goodbye.
  std::cout << "5/5 Leaving main()." << std::endl;
  std::cout << "Goodbye." << std::endl;
  return 0;
}
