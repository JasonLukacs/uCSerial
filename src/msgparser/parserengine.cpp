#include "uCSerial/msgparser/parser_engine.h"
#include "uCSerial/utils/utils.h"

#include <jsonparser.h>

#include <iostream>
#include <regex>

// Main loop
bool ParserEngine::run() {
  loadRules();
  this->serialReader = std::make_unique<SerialReader>();

  try {
    bufferSize = serialReader->Run(
        path, [this]() { onSerialDataAvailable(); },
        [this](const std::string &error_message) {
          onSerialError(error_message);
        });
  } catch (const SerialReaderException &e) {
    onSerialError(e.what());
  }

  return true;
}

// private:

void ParserEngine::onSerialDataAvailable() {
  // Get data
  std::vector<char> buffer(bufferSize);
  int bytesRead = serialReader->Read(buffer);

  using enum State;
  for (char currentByte :
       std::vector<char>(buffer.begin(), buffer.begin() + bytesRead)) {
    switch (currentState) {
    case READING_MSG_START:
      readMsgStart(currentByte);
      break;
    case READING_VALUE_START:
      readValueStart(currentByte);
      break;
    case READING_VALUE_TYPE:
      readValueType(currentByte);
      break;
    case READING_VALUE:
      readValue(currentByte);
      break;
    }
  }
}

bool ParserEngine::stop() const {
  // Stop reading serial port.
  serialReader->Stop();
  std::cout << "4/6 Parser engine finished." << std::endl;
  return true;
}

bool ParserEngine::onSerialError(const std::string &errorMessage) const {
  callbackOnError(errorMessage);
  return true;
}

bool ParserEngine::readMsgStart(char char_in) {
  if (char_in == msgRules.startMsg) {
    currentState = State::READING_VALUE_START;
  }

  return true;
}

bool ParserEngine::readValueStart(char charIn) {
  if (charIn == msgRules.startValue) {
    currentState = State::READING_VALUE_TYPE;
  } else if (charIn == msgRules.stopMsg) {
    std::cout << std::endl;
    currentState = State::READING_MSG_START;
  }

  return true;
}

bool ParserEngine::readValueType(char charIn) {
  if (valueTypeBuffer.length() < 3) {
    valueTypeBuffer += charIn;
  }

  if (valueTypeBuffer.length() == 3) {
    if (msgRules.valueRules.contains(valueTypeBuffer)) {
      valueType = valueTypeBuffer;
      currentState = State::READING_VALUE;
    } else {
      currentState = State::READING_MSG_START;
    }

    valueTypeBuffer = "";
  }
  return true;
}

bool ParserEngine::readValue(char charIn) {
  if (charIn == msgRules.stopValue && validateValue()) {
    // Received valid value.
    std::cout << valueType << ": " << valueBuffer << "\t ";
    valueBuffer = "";
    currentState = State::READING_VALUE_START;
  } else if (valueBuffer.length() <
                 msgRules.valueRules.at(valueType).maxDigits &&
             std::isdigit(charIn)) {
    // Received valid character for value buffer.
    valueBuffer += charIn;
  } else {
    // Unexpected character, ignore message;
    // std::cout << "Received invalid char while reading value." << std::endl;
    currentState = State::READING_MSG_START;
  }

  return true;
}

bool ParserEngine::validateValue() const {
  int32_t receivedValue = 0;

  // Test if current string buffer contains an int.
  if (std::regex_match(valueBuffer, std::regex("^[-]?[0-9]+$"))) {
    // If an integer, test against the appropriate rules for this message type.
    if (receivedValue >= msgRules.valueRules.at(valueType).minValue &&
        receivedValue <= msgRules.valueRules.at(valueType).maxValue) {
      return true;
    } else {
      // not
    }
  } else {
    // not
  }

  return true;
}

bool ParserEngine::loadRules() {
  std::string rules_JSON_schema =
      "../include/uCSerial/msgparser/rules_schema.json";
  std::string rules_file = "../config/msg_rules.json";

  // Validate config file, schema, and config file against schema.
  try {
    JSONParser JSONparser;
    JSONparser.JSONValidate(rules_JSON_schema, rules_file);
  } catch (const JSONParserException &e) {
    onSerialError(e.what());
  }

  // Validated, safe to create a rapidjson document.
  JSONParser JSONparser;
  rapidjson::Document document = JSONparser.GetJSONDocument(rules_file);

  // Populate return object
  msgRules.startMsg = document["startMsg"].GetString()[0];
  msgRules.stopMsg = document["stopMsg"].GetString()[0];
  msgRules.startValue = document["startValue"].GetString()[0];
  msgRules.stopValue = document["stopValue"].GetString()[0];

  if (document.HasMember("valueRules") && document["valueRules"].IsObject()) {
    const rapidjson::Value &valueRules = document["valueRules"];

    for (const auto &[key, innerObject] : valueRules.GetObject()) {
      std::string msgDescriptor = innerObject["msgDescriptor"].GetString();
      int minValue = innerObject["minValue"].GetInt();
      int maxValue = innerObject["maxValue"].GetInt();
      uint8_t digit_count =
          std::max(countDigits(minValue), countDigits(maxValue));
      msgRules.valueRules.try_emplace(key.GetString(), msgDescriptor, minValue,
                                      static_cast<uint32_t>(maxValue),
                                      digit_count);
    }
  } else {
    // Handle error: "valueRules" not found or is not an object
  }

  return true;
}

template <typename T> uint8_t ParserEngine::countDigits(T x) const {
  uint8_t digits = x == 0 ? 1 : static_cast<uint8_t>(log10(abs(x))) + 1;
  digits += (x < 0 ? 1 : 0);
  return digits;
}
