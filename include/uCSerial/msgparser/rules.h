#ifndef RULES_H
#define RULES_H

#include <map>
#include <string>

class Rules {
  public:
    char startMsg = '\0';
    char stopMsg = '\0';
    char startValue = '\0';
    char stopValue = '\0';

    struct ValueRules {
        std::string msgDescriptor;
        int32_t minValue;
        uint32_t maxValue;
        uint8_t maxDigits;
    };

    std::map<std::string, ValueRules> valueRules = {};

  private:
};


#endif