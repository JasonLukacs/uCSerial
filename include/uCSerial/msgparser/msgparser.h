#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "uCSerial/msgparser/exceptions.h"
#include "uCSerial/msgparser/parser_engine.h"

class MessageParser {
  public:
    explicit MessageParser(const std::string &path) : path(path), parserEngine(path) {}
    bool Run();

  private:
    const std::string path;
    std::array<int, 2> pipefd;
    ParserEngine parserEngine;

    bool Stop(const std::string &error_message) const;
};

#endif