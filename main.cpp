/*
class               PascalCase
object/pointer      camelCase
variable            camelCase
function            camelCase

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
  messageParser.run();

  // Goodbye.
  std::cout << "6/6 Leaving main()." << std::endl;
  std::cout << "Goodbye." << std::endl;
  
  return 0;
  
}