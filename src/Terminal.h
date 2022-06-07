#ifndef TERMINAL_INCLUDED
#define TERMINAL_INCLUDED

#include "Stream.h"

#define term_in _termIn()
#define term_out _termOut()
#define term_err _termErr()

#define term_COLRESET "\x1b[0m"

#define term_BBLACK "\x1b[90m"
#define term_BRED "\x1b[91m"
#define term_BGREEN "\x1b[92m"
#define term_BYELLOW "\x1b[93m"
#define term_BBLUE "\x1b[94m"
#define term_BMAGENTA "\x1b[95m"
#define term_BCYAN "\x1b[96m"
#define term_BWHITE "\x1b[97m"

Stream* _termIn();
Stream* _termOut();
Stream* _termErr();

#endif // TERMINAL_INCLUDED