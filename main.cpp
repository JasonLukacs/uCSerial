/*
class               PascalCase
object/pointer      camelCase
variable            snake_case
function            PascalCase

*/

#include <filesystem>
#include <iostream>

#include "uCSerial/msgparser/msgparser.h"

int main(int argc, char *argv[]) {
  // Fetch path to find config files.
  std::filesystem::path executable(argv[0]);
  std::string path = executable.parent_path().string();

  // Run the message parser.
  MessageParser messageParser(path);
  try {
    messageParser.Run();
  } catch (const MsgParserException &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // Goodbye.
  std::cout << "6/6 Leaving main()." << std::endl;
  std::cout << "Goodbye." << std::endl;
  return 0;
}