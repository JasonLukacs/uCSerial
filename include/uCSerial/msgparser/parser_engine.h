#ifndef PARSERENGINE_H
#define PARSERENGINE_H

#include "uCSerial/msgparser/rules.h"
#include "uCSerial/serialreader/serialreader.h"

class ParserEngine {
public:
  explicit ParserEngine(const std::string &path,
                        const std::function<void(std::string)> &callback)
      : path(path), callbackOnError(callback) {}

  bool run();
  bool stop() const;

private:
  // Program (-flow)
  const std::string path;
  std::function<void(std::string)> callbackOnError;
  int bufferSize = 0;

  enum class State {
    READING_MSG_START,
    READING_VALUE_START,
    READING_VALUE_TYPE,
    READING_VALUE
  };
  State currentState = State::READING_MSG_START;

  Rules msgRules;
  std::unique_ptr<SerialReader> serialReader;

  bool loadRules();
  template <typename T> uint8_t countDigits(T x) const;

  void onSerialDataAvailable();
  bool onSerialError(const std::string &errorMessage) const;

  // State handling
  std::string valueType;
  std::string valueTypeBuffer;
  std::string valueBuffer;

  bool readMsgStart(char charIn);
  bool readValueStart(char charIn);
  bool readValueType(char charIn);
  bool readValue(char charIn);
  bool validateValue() const;
};

#endif