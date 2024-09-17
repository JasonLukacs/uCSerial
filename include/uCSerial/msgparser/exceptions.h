#include <string>

class MsgParserException : public std::runtime_error {
    using runtime_error::runtime_error;
};
