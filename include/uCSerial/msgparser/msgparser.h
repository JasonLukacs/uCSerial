#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "exceptions.h"
#include "uCSerial/serialreader/serialreader.h"

class MessageParser {
public:
    MessageParser() = default;
    bool Run();
    bool ReadData(SerialReader::ReadResult result) const;

private:
    int buffer_size = 0;
    
    std::unique_ptr<SerialReader> serialReader;
    
    bool Stop();
    void PrintResult() const;
};

#endif