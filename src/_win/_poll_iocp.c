#include "cel/poll.h"
#ifdef _CEL_WIN
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /*cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args) /*cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args) /*cel_log_err args*/

int cel_poll_init(CelPoll *poll, int max_threads, int max_fileds)
{
    return (poll->CompletionPort = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE, NULL, 0, max_threads)) != INVALID_HANDLE_VALUE ? 0 : -1;
}

void cel_poll_destroy(CelPoll *poll)
{
    CloseHandle(poll->CompletionPort);
}

int cel_poll_wait(CelPoll *poll, CelOverLapped **ol, int milliseconds)
{
    DWORD nbytes;
    ULONG_PTR key;

    *ol = NULL;
    if (!GetQueuedCompletionStatus(
        poll->CompletionPort, &nbytes, &key, (LPOVERLAPPED *)ol, milliseconds)
        && WSAGetLastError() == WAIT_TIMEOUT)
        return 0;
    /*
    _tprintf(_T("bytes = %ld, %p, erro %d, pid %d\r\n"), 
        nbytes, ol, WSAGetLastError(), (int)cel_thread_getid());
        */
    if (*ol != NULL)
    {
        *ol = (CelOverLapped *)((char *)*ol - CEL_OFFSET(CelOverLapped, _ol));
        if (CEL_CHECKFLAG((*ol)->evt_type, CEL_EVENT_CHANNEL))
        {
            (*ol)->result = nbytes;
            (*ol)->key = (void *)key;
            (*ol)->error = WSAGetLastError();
        }
        return 0;
    }
    return -1;
}

#endif
