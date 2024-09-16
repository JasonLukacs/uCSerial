#ifndef uCSERIALUTILS_H
#define uCSERIALUTILS_H

namespace uCSerialUtils {
    bool WaitForKeypress(std::array<int, 2> pipefd);
}

#endif