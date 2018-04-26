#include "cel/poll.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /*cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args) /*cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args) /*cel_log_err args*/

CelPoll *cel_poll_new(int max_threads, int max_fileds)
{
    CelPoll *poll;

    if ((poll = (CelPoll *)cel_malloc(sizeof(CelPoll))) != NULL)
    {
        if (cel_poll_init(poll, max_threads, max_fileds) == 0)
            return poll;
        cel_free(poll);
    }
    return NULL;
}

void cel_poll_free(CelPoll *poll)
{
    cel_poll_destroy(poll);
    cel_free(poll);
}

