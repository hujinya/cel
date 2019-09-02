#include "cel/error.h"
#include "cel/net/socket.h"

int poll_test(int argc, TCHAR *argv[])
{
//    CelPoll poll;
//    CelSocket listen_sock;
//    CelSocketAsyncAcceptArgs arg, *parg;
//    char buf[1024];
//    CelOverLapped *ol;
//    CelSockAddr addr;
//
//    if (cel_poll_init(&poll, 10, 1024) == -1)
//    {
//        _tprintf(_T("poll %d\r\n"), cel_geterrno());
//        return -1;
//    }
//#ifdef _CEL_WIN
//    cel_wsastartup();
//#endif
//    if (cel_socket_init(&listen_sock, AF_INET, SOCK_STREAM, IPPROTO_TCP) == -1
//        || cel_socket_listen_host(&listen_sock, _T("0.0.0.0"), 9060, 1024) == -1
//        || cel_socket_set_nonblock(&listen_sock, 1) == -1)
//    {
//        _tprintf(_T("listen_sock %s\r\n"), cel_geterrstr());
//        return -1;
//    }
//    _tprintf(_T("Listen %s.\r\n"), 
//        cel_sockaddr_ntop(cel_socket_get_localaddr(&listen_sock, &addr)));
//
//    if (cel_poll_add(&poll, (CelChannel *)&(listen_sock), 0) == -1)
//    {
//        _tprintf(_T("associate %s\r\n"), cel_geterrstr());
//        return -1;
//    }
//
//    memset(&arg, 0, sizeof(CelSocketAsyncAcceptArgs));
//    memcpy(&arg.socket, &listen_sock, sizeof(CelSocket));
//    arg.buffer = buf;
//    arg.buffer_size = 1024;
//    if (cel_socket_async_accept(&arg) == -1)
//    {
//        _tprintf(_T("post %s\r\n"), cel_geterrstr());
//        return -1;
//    }
//    while (cel_poll_wait(&poll, &ol, -1) != -1)
//    {
//        _tprintf(_T("%p\r\n"), ol);
//        parg = (CelSocketAsyncAcceptArgs *)ol;
//#ifdef _CEL_WIN
//        cel_socket_update_acceptcontext(&(parg->accept_socket), parg->socket);
//#endif
//        if (cel_socket_get_remoteaddr(&parg->accept_socket, &addr) == NULL)
//            _putts(cel_geterrstr());
//        _tprintf(_T("Accept %s %d\r\n"), cel_sockaddr_ntop(&addr), parg->accept_socket.fd);
//        cel_socket_destroy(&parg->accept_socket);
//
//        memcpy(&arg.socket, &listen_sock, sizeof(CelSocket));
//        arg.buffer = buf;
//        arg.buffer_size = 1024;
//        _tprintf(_T("post %p\r\n"), &arg);
//        if (cel_socket_async_accept(&arg) == -1)
//        {
//            _tprintf(_T("post %s\r\n"), cel_geterrstr());
//            return -1;
//        }
//    }

    return 0;
}
