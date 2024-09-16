/*
class               PascalCase
object/pointer      camelCase
variable            snake_case
function            PascalCase

*/

#include <filesystem>
#include <iostream>

#include "uCSerial/msgparser/msgparser.h"

std::string GetPath(char *_argv0);

int main(int argc, char *argv[]) {
  std::string path = "";
  path = GetPath(argv[0]);

  // Run the message parser.
  MessageParser messageParser(path);
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

std::string GetPath(char *_argv0) {
  std::filesystem::path executablePath(_argv0);
  std::filesystem::path executablePathWithoutFilename = executablePath.parent_path();

  return executablePathWithoutFilename.string();
}
