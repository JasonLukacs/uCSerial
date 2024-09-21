#ifndef SERIALREADER_EXCEPTIONS_H
#define SERIALREADER_EXCEPTIONS_H

#include <string>

class SerialReaderException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

#endif