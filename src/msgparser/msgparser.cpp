#include "uCSerial/msgparser/msgparser.h"

#include <fstream>
#include <iostream>
#include <jsonparser.h>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>


#include "uCSerial/serialreader/serialreader.h"
#include "uCSerial/utils/utils.h"

// Main loop
bool MessageParser::Run() {
    LoadRules();

    // Start monitoring the serial port
    this->serialReader = std::make_unique<SerialReader>(path);

    try {
        serialReader->InitSerial();
        buffer_size = serialReader->GetBufferSize();
        serialReader->StartReadingPort([this](SerialReader::ReadResult result) { ReadData(result); });

    } catch (const SerialReaderException &e) {
        throw MsgParserException(e.what());
    }

    // Run untill any key is pressed or pipe is written to.
    if (pipe(pipefd.data()) == -1) {
        std::string error_message = "Failed to create pipe. ";
        throw MsgParserException(error_message);
    }
    SerialUtils::WaitForKeypress(pipefd);

    // Stop parser.
    Stop();

    return true;
}


// Callback function for SerialReader.
bool MessageParser::ReadData(SerialReader::ReadResult result) {
    using enum SerialReader::ReadResult;

    switch (result) {
    case READ_SUCCESS:
        ParseData();
        break;
    case READ_TIMEOUT:
        std::cout << "Serial timed out." << std::endl;
        ForceExit();
        break;
    default:
        std::cout << "Serial error." << std::endl;
        ForceExit();
        break;
    }

    return true;
}


// private:

void MessageParser::ParseData() {
    // Get data
    std::vector<char> buffer(buffer_size);
    int bytesRead = serialReader->ReadToBuffer(buffer);

    using enum State;

    for (char currentByte : std::vector<char>(buffer.begin(), buffer.begin() + bytesRead)) {
        switch (currentState) {
        case READING_MSG_START:
            ReadMsgStart(currentByte);
            break;
        case READING_VALUE_START:
            ReadValueStart(currentByte);
            break;
        case READING_VALUE_TYPE:
            ReadValueType(currentByte);
            break;
        case READING_VALUE:
            ReadValue(currentByte);
            break;
        }
    }
}


void MessageParser::PrintResult() const {
    std::vector<char> buffer(buffer_size);
    int bytes_read = serialReader->ReadToBuffer(buffer);
    std::cout << "Bytes read:      " << bytes_read << std::endl;
    std::cout << "Data received:" << std::endl;
    for (int i = 0; i < bytes_read; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
}


bool MessageParser::Stop() const {
    // Stop reading serial port.
    serialReader->StopReadingPort();
    std::cout << "4/5 MessageParser finished." << std::endl;
    return true;
}


bool MessageParser::ForceExit() const {
    // Write to the pipe monitored by uCSerialUtils::WaitForKeypress to trigger poll()
    write(pipefd[1], "x", 1);

    return true;
}


bool MessageParser::ReadMsgStart(char char_in) {
    if (char_in == msg_rules.startMsg) {
        currentState = State::READING_VALUE_START;
    }

    return true;
}


bool MessageParser::ReadValueStart(char charIn) {
    if (charIn == msg_rules.startValue) {
        currentState = State::READING_VALUE_TYPE;
    } else if (charIn == msg_rules.stopMsg) {
        std::cout << std::endl;
        currentState = State::READING_MSG_START;
    }

    return true;
}


bool MessageParser::ReadValueType(char charIn) {
    if (valueTypeBuffer.length() < 3) {
        valueTypeBuffer += charIn;
    }

    if (valueTypeBuffer.length() == 3) {
        if (msg_rules.valueRules.contains(valueTypeBuffer)) {
            valueType = valueTypeBuffer;
            currentState = State::READING_VALUE;
        } else {
            currentState = State::READING_MSG_START;
        }

        valueTypeBuffer = "";
    }
    return true;
}


bool MessageParser::ReadValue(char charIn) {
    if (charIn == msg_rules.stopValue && ValidateValue()) {
        // Received valid value.
        std::cout << valueType << ": " << valueBuffer << ", ";
        valueBuffer = "";
        currentState = State::READING_VALUE_START;
    } else if (valueBuffer.length() < msg_rules.valueRules.at(valueType).maxDigits && std::isdigit(charIn)) {
        // Received valid character for value buffer.
        valueBuffer += charIn;
    } else {
        // Unexpected character, ignore message;
        //std::cout << "Received invalid char while reading value." << std::endl;
        currentState = State::READING_MSG_START;
    }

    return true;
}


bool MessageParser::ValidateValue() const {
    int32_t receivedValue = 0;

    // Test if current string buffer contains an int.
    if (std::regex_match(valueBuffer, std::regex("^[-]?[0-9]+$"))) {
        // If an integer, test against the appropriate rules for this message type.
        if (receivedValue >= msg_rules.valueRules.at(valueType).minValue && receivedValue <= msg_rules.valueRules.at(valueType).maxValue) {
            return true;
        } else {
            // not
        }
    } else {
        // not
    }

    return true;
}


bool MessageParser::LoadRules() {
    std::string rules_JSON_schema = "../include/uCSerial/msgparser/rules_schema.json";
    std::string rules_file = "../config/msg_rules.json";

    // Validate config file, schema, and config file against schema.
    try {
        JSONParser JSONparser;
        JSONparser.JSONValidate(rules_JSON_schema, rules_file);
    } catch (const JSONParserException &e) {
        throw MsgParserException(e.what());
    }

    // Validated, safe to create a rapidjson document.
    JSONParser JSONparser;
    rapidjson::Document document = JSONparser.GetJSONDocument(rules_file);

    // Populate return object
    msg_rules.startMsg = document["startMsg"].GetString()[0];
    msg_rules.stopMsg = document["stopMsg"].GetString()[0];
    msg_rules.startValue = document["startValue"].GetString()[0];
    msg_rules.stopValue = document["stopValue"].GetString()[0];

    if (document.HasMember("valueRules") && document["valueRules"].IsObject()) {
        const rapidjson::Value &valueRules = document["valueRules"];

        for (const auto &[key, innerObject] : valueRules.GetObject()) {
            std::string msgDescriptor = innerObject["msgDescriptor"].GetString();
            int minValue = innerObject["minValue"].GetInt();
            int maxValue = innerObject["maxValue"].GetInt();
            uint8_t digit_count = std::max(CountDigits(minValue), CountDigits(maxValue));
            msg_rules.valueRules.try_emplace(key.GetString(), msgDescriptor, minValue, static_cast<uint32_t>(maxValue), digit_count);
        }
    } else {
        // Handle error: "valueRules" not found or is not an object
    }


    return true;
}


template <typename T>
uint8_t MessageParser::CountDigits(T x) const {
    if (x == 0)
        return 1;
    if (x < 0)
        x = -x;

    uint8_t count = 0;
    while (x > 0) {
        x /= 10;
        count++;
    }
    return count;
}
