#include <array>
#include <poll.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "uCSerial/utils/utils.h"

bool uCSerialUtils::WaitForKeypress(std::array<int, 2> pipefd) {
    struct termios old_termios;
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ICANON;
    // Disable canonical mode
    new_termios.c_lflag &= ~ECHO; // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    std::array<pollfd, 2> fds;
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = pipefd[0];
    fds[1].events = POLLIN;

    poll(fds.data(), 2, -1);

    // Flush to avoid spill over of keyboard input
    // to terminal after termination.
    int bytes_available;
    std::string buffer;
    ioctl(STDIN_FILENO, FIONREAD, &bytes_available);
    read(STDIN_FILENO, &buffer, bytes_available);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    return true;
};