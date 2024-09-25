#ifndef PARSERENGINE_H
#define PARSERENGINE_H

#include "uCSerial/msgparser/rules.h"
#include "uCSerial/serialreader/serialreader.h"

class ParserEngine {
  public:
    explicit ParserEngine(const std::string &path) : path(path) {}
   
    bool Run(const std::function<void(std::string)> &callback_function);
    bool Stop() const;

  private:
    // Program (-flow)
    const std::string path;
    int buffer_size = 0;

    enum class State {
        READING_MSG_START,
        READING_VALUE_START,
        READING_VALUE_TYPE,
        READING_VALUE
    };
    State currentState = State::READING_MSG_START;

    Rules msg_rules;
    std::unique_ptr<SerialReader> serialReader;

    bool LoadRules();
    template <typename T>
    uint8_t CountDigits(T x) const;

    void onDataAvailable();
    bool onError(std::string error_message) const;
  
    // State handling
    std::string valueType;
    std::string valueTypeBuffer;
    std::string valueBuffer;

    bool ReadMsgStart(char char_in);
    bool ReadValueStart(char charIn);
    bool ReadValueType(char charIn);
    bool ReadValue(char charIn);
    bool ValidateValue() const;
};

#endif