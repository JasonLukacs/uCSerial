#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "exceptions.h"
#include "uCSerial/msgparser/parser_engine.h"

class MessageParser {
  public:
    explicit MessageParser(const std::string &path) : path(path) {}
    bool Run();

  private:
    std::string path;
    std::array<int, 2> pipefd;

    ParserEngine parserEngine;

    bool ForceExit() const;

};

#endif