#include <iostream>
#include <termios.h>
#include <unistd.h>

#include "uCSerial/utils/utils.h"

bool uCSerialUtils::WaitForKeypress() {
    struct termios old_termios;
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ICANON;
    // Disable canonical mode
    new_termios.c_lflag &= ~ECHO;  // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int ret;
    std::string buffer;

    while (true) {
        ret = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, nullptr);
        if (ret > 0) {
            // Flush to avoid spill over of keyboard input to terminal after termination.
            read(STDIN_FILENO, &buffer, 1);
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    return true;
};