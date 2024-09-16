#include <array>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <string>

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

  while (true) {
    std::array<pollfd, 2> fds;
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = pipefd[0];
    fds[1].events = POLLIN;

    int ret = poll(fds.data(), 2, -1);
    if (ret > 0) {
      if (fds[0].revents & POLLIN || fds[1].revents & POLLIN) {
        // Flush to avoid spill over of keyboard input
        // to terminal after termination.
        int bytes_available;
        std::string buffer;
        ioctl(STDIN_FILENO, FIONREAD, &bytes_available);
        read(STDIN_FILENO, &buffer, bytes_available);
        break;
      }
    } else if (ret == 0) {
      // Timeout (not applicable in this case)
    } else {
      // Error
      // Handle error as needed
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
  return true;
};