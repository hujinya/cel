#include "cel/buffer.h"
#include "cel/multithread.h"
#include "cel/thread.h"

int read_thread(void *data)
{
    int i, j;
    char out_data[127];
    CelBuffer *buf = (CelBuffer *)data;

    i = cel_buffer_read(buf, out_data, 30);
    _tprintf(_T("Ring buffer read %d\r\n"),i);
    for (j = 0; j < i;j++)
    {
        _tprintf(_T("%d "),out_data[j]);
    }
    _tprintf(_T("\r\n"));
    i = cel_buffer_read(buf, out_data, 127);
    _tprintf(_T("Ring buffer read %d\r\n"),i);
    for (j = 0; j < i;j++)
    {
        _tprintf(_T("%d "),out_data[j]);
    }
    _tprintf(_T("\r\n"));
    cel_thread_exit(0);

    return 0;
}

int write_thread(void *data)
{
    int i;
    char in_data[127];
    CelBuffer *buf = (CelBuffer *)data;

    for (i = 0;i < 127;i++)
    {
        in_data[i] = i;
    }
    i = cel_buffer_write(buf, in_data, 60);
    _tprintf(_T("Ring buffer wirte %d\r\n"),i);
    i = cel_buffer_write(buf, in_data, 120);
    _tprintf(_T("Ring buffer wirte %d\r\n"),i);
    cel_thread_exit(0);

    return 0;
}

int buffer_test(int argc, TCHAR *argv[])
{
    int i, j;
    char out_data[127];
    CelBuffer *buf;
    CelThread td[2];

    cel_multithread_support();
    buf = cel_buffer_new(127);
    cel_thread_create(&td[0], NULL, write_thread, buf);
    cel_thread_create(&td[1], NULL, read_thread, buf);

    //sleep(1);
    cel_thread_join(td[0], NULL);
    cel_thread_join(td[1], NULL);
    i = cel_buffer_read(buf, out_data, 127);
    _tprintf(_T("Ring buffer read %d\r\n"),i);
    for (j = 0; j < i;j++)
    {
        _tprintf(_T("%d "),out_data[j]);
    }
    _tprintf(_T("\r\n"));

    return 0;
}
