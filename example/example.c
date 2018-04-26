#include "cel/types.h"

extern int aes_test(int argc, TCHAR *argv[]);
extern int allocator_test(int argc, TCHAR *argv[]);
extern int arraylist_test(int argc, TCHAR *argv[]);
extern int buffer_test(int argc, TCHAR *argv[]);
extern int command_test(int argc, TCHAR *argv[]);
extern int convert_test(int argc, TCHAR *argv[]);
extern int coroutine_test(int argc, TCHAR *argv[]);
extern int des_test(int argc, TCHAR *argv[]);
extern int error_test(int argc, TCHAR *argv[]);
extern int eventloop_test(int argc, TCHAR *argv[]);
extern int file_test(int argc, TCHAR *argv[]);
extern int hacluster_test(int argc, TCHAR *argv[]);
extern int hashtable_test(int argc, TCHAR *argv[]);
extern int httpclient_test(int argc, TCHAR *argv[]);
extern int httplistener_test(int argc, TCHAR *argv[]);
extern int json_test(int argc, TCHAR *argv[]);
extern int jwt_test(int argc, TCHAR *argv[]);
extern int log_test(int argc, TCHAR *argv[]);
extern int md5_test(int argc, TCHAR *argv[]);
extern int minheap_test(int argc, TCHAR *argv[]);
extern int monitor_test(int argc, TCHAR *argv[]);
extern int readline_test(int argc, TCHAR *argv[]);
extern int rbtree_test(int argc ,const char *argv[]);
extern int perf_test(int argc, TCHAR *argv[]);
extern int poll_test(int argc, TCHAR *argv[]);
extern int process_test(int argc, TCHAR *argv[]);
extern int profile_test(int argc, TCHAR *argv[]);
extern int profiles_test(int argc, TCHAR *argv[]);
extern int queue_test(int argc, TCHAR *argv[]);
extern int socket_test(int argc, TCHAR *argv[]);
extern int sslsocket_test(int argc, TCHAR *argv[]);
extern int tcplistener_test(int argc, TCHAR *argv[]);
extern int terminal_test(int argc, TCHAR *argv[]);
extern int threadpool_test(int argc, TCHAR *argv[]);
extern int timerheap_test(int argc ,const char *argv[]);
extern int timerwheel_test(int argc ,const char *argv[]);
extern int user_test(int argc, TCHAR *argv[]);
extern int vrrp_test(int argc, TCHAR *argv[]);
extern int wmipclient_test(int argc, TCHAR *argv[]);

extern int devredirclient_test(int argc, TCHAR *argv[]);

int _tmain(int argc, TCHAR *argv[])
{
    //_tscanf(_T("%s", ))

    return jwt_test(argc, argv);
}
