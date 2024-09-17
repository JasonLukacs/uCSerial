#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "exceptions.h"
#include "uCSerial/serialreader/serialreader.h"

class MessageParser {
  public:
    explicit MessageParser(const std::string &path) : path(path) {}

    bool Run();
    bool ReadData(SerialReader::ReadResult result) const;

  private:
    std::string path;
    int buffer_size = 0;
    std::array<int, 2> pipefd;

    std::unique_ptr<SerialReader> serialReader;

    bool Stop() const;
    void PrintResult() const;
    bool ForceExit() const;
};

#endif