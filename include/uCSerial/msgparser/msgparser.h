#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "uCSerial/msgparser/parser_engine.h"

class MessageParser {
  public:
    explicit MessageParser(const std::string &path) : path(path), parserEngine(path) {}
    bool Run();

  private:
    const std::string path;
    std::array<int, 2> exit_pipe;
    ParserEngine parserEngine;

    bool ForceExit(const std::string &error_message) const;
};

#endif