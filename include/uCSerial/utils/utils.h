#ifndef uCSERIALUTILS_H
#define uCSERIALUTILS_H

#include <array>

class SerialUtils {
public:
  static bool waitForKeypress(std::array<int, 2> pipefd);
};

#endif