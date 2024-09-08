/*
class               PascalCase
object/pointer      camelCase
variable            snake_case
function            PascalCase

*/

#include <iostream>

#include "uCSerial/msgparser/msgparser.h"
#include "uCSerial/utils/utils.h"

int main() {
    // Start parser.
    MessageParser messageParser;

    try {
        messageParser.Start();
    } catch (const MsgParserException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Run untill any key is pressed.
    uCSerialUtils::WaitForKeypress();

    // Terminate.
    messageParser.Stop();

    return 0;
}
