#include "cel/error.h"
#ifdef _CEL_UNIX
WCHAR *os_strerror_w(int err_no)
{
    return NULL;
}
#endif
