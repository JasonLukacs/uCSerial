#ifndef uCSERIALUTILS_H
#define uCSERIALUTILS_H

class SerialUtils {
public:
    static bool WaitForKeypress(std::array<int, 2> pipefd);
};

#endif