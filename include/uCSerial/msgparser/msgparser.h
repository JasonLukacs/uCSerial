#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "exceptions.h"
#include "uCSerial/serialreader/serialreader.h"

class MessageParser {
public:
    MessageParser() = default;
    bool Start();
    bool Stop();
    bool ReadData(SerialReader::ReadResult result) const;

private:
    std::unique_ptr<SerialReader> serialReader;
    void PrintResult() const;
};

#endif