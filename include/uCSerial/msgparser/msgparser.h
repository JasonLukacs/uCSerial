#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "uCSerial/msgparser/parser_engine.h"

class MessageParser {
public:
  explicit MessageParser(const std::string &path)
      : path(path), parserEngine(path) {}
  bool run();

private:
  const std::string path;
  std::array<int, 2> exitPipe;
  ParserEngine parserEngine;

  bool forceExit(const std::string &errorMessage) const;
};

#endif