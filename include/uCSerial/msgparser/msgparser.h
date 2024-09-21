#ifndef MSGPARSER_H
#define MSGPARSER_H

#include "exceptions.h"
#include "uCSerial/msgparser/rules.h"
#include "uCSerial/serialreader/serialreader.h"

class MessageParser {
  public:
    explicit MessageParser(const std::string &path) : path(path) {}

    bool Run();
    bool ReadData(SerialReader::ReadResult result);

  private:
    enum class State {
        READING_MSG_START,
        READING_VALUE_START,
        READING_VALUE_TYPE,
        READING_VALUE
    };
    State currentState = State::READING_MSG_START;

    std::string path;
    int buffer_size = 0;
    std::array<int, 2> pipefd;

    Rules msg_rules;
    std::unique_ptr<SerialReader> serialReader;

    // Program (-flow)
    bool LoadRules();
    template <typename T>
    uint8_t CountDigits(T x) const;

    void ParseData();
  
    bool Stop() const;
    bool ForceExit() const;

    // State handlers
    bool ReadMsgStart(char char_in);
    bool ReadValueStart(char charIn);

    std::string valueType;
    std::string valueTypeBuffer;
    bool ReadValueType(char charIn);

    std::string valueBuffer;
    bool ReadValue(char charIn);

    // Helper
    bool ValidateValue() const;
};

#endif