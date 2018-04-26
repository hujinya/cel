#include "cel/types.h"
#ifdef __UNIX__
#include <termios.h>

int terminal_test(int argc, TCHAR *argv[])
{
    TCHAR ch;

    struct termios state;
    tcgetattr(STDIN_FILENO,&state);
    state.c_lflag &= ~(ECHO|ICANON);
    state.c_cc[VMIN]=1;              
    state.c_cc[VTIME]=0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &state);
    while (TRUE)
    {
        _tscanf(_T("%c"), &ch);
        _puttchar(ch);
    }
    return 0;
}
#endif
