#ifndef MSGPARSER_EXCEPTIONS_H
#define MSGPARSER_EXCEPTIONS_H

#include <string>

class MsgParserException : public std::runtime_error {
    using runtime_error::runtime_error;
};

#endif