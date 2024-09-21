#ifndef PARSERENGINE_H
#define PARSERENGINE_H

#include "exceptions.h"
#include "uCSerial/msgparser/rules.h"
#include "uCSerial/serialreader/serialreader.h"

class ParserEngine {
  public:
    
    bool Run(const std::string &path);
    bool Stop() const;

  private:
    // Program (-flow)
    enum class State {
        READING_MSG_START,
        READING_VALUE_START,
        READING_VALUE_TYPE,
        READING_VALUE
    };
    State currentState = State::READING_MSG_START;

    std::string path;
    int buffer_size = 0;

    Rules msg_rules;
    std::unique_ptr<SerialReader> serialReader;

    bool LoadRules();
    template <typename T>
    uint8_t CountDigits(T x) const;

    bool ReadData(SerialReader::ReadResult result);
    void ParseData();
  
    // State handlers
    std::string valueType;
    std::string valueTypeBuffer;
    std::string valueBuffer;

    bool ReadMsgStart(char char_in);
    bool ReadValueStart(char charIn);
    bool ReadValueType(char charIn);
    bool ReadValue(char charIn);
    bool ValidateValue() const;  // Helper
};

#endif