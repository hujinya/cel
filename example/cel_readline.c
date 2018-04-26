#include "cel/_readline.h"
#include "cel/error.h"
#include "cel/net/tcplistener.h"

int readline_test(int argc, TCHAR *argv[])
{
    //BOOL client_active = FALSE;
    //CelTcpListener listener;
    //CelTcpClient new_client, client;
    //CelSockAddr addr;
    //CelReadLine vty;
    //char buf[1024];
    //int r_size, w_size;

#ifdef _CEL_WIN
    cel_wsastartup();
#endif
 /*   if (cel_sockaddr_init_ipstr(&addr, _T("0.0.0.0"), 9095) != 0
        || cel_tcplistener_init(&listener, &addr) != 0
        || cel_tcplistener_set_reuseaddr(&listener, 1) != 0
        || cel_tcplistener_set_nonblock(&listener, 1) != 0)
    {
        _tprintf(_T("%s\r\n"), cel_geterrstr(cel_geterrno()));
        return -1;
    }
    _tprintf(_T("Server %s listenning.\r\n"), 
        cel_tcplistener_get_localaddrs(&listener));

    while (TRUE)
    {
        if (cel_tcplistener_accept(&listener, &new_client) == 0)
        {
            if (client_active)
                cel_tcpclient_destroy(&client);
            memcpy(&client, &new_client, sizeof(CelTcpClient));
            cel_tcpclient_set_nonblock(&client, 1);
            client_active = TRUE;
            _tprintf(_T("Client %s:%d connect.\r\n"), 
                cel_tcpclient_get_remoteaddrs(&client), client.sock.fd);
            cel_readline_init(&vty);
        }
        if (client_active)
        {
            if ((r_size = cel_tcpclient_recv(&client, buf, 1024)) <= 0)
            {
                if (r_size == 0 
                    || (r_size < 0 && cel_geterrno() != EWOULDBLOCK && cel_geterrno() != EAGAIN))
                {
                    _tprintf(_T("Client %s disconnect,read(%s).\r\n"), 
                        cel_tcpclient_get_remoteaddrs(&client), 
                        cel_geterrstr(cel_geterrno()));
                    cel_tcpclient_destroy(&client);
                    client_active = FALSE;
                }
                r_size = 0;
            }
            else
            {
                _tprintf(_T("Read %d\r\n"), r_size);
                cel_readline_in(&vty, buf, r_size);
            }
            if (vty.out_cursor > 0)
            {
                if ((w_size = cel_tcpclient_send(
                    &client, (char *)vty.out_buf, vty.out_cursor)) <= 0)
                {
                    if (w_size == 0  
                        || (w_size < 0 && cel_geterrno() != EWOULDBLOCK && cel_geterrno() != EAGAIN))
                    {
                        _tprintf(_T("Client %s disconnect,write(%s).\r\n"), 
                            cel_tcpclient_get_remoteaddrs(&client),
                            cel_geterrstr(cel_geterrno()));
                        cel_tcpclient_destroy(&client);
                        client_active = FALSE;
                    }
                    w_size = 0;
                }
                else 
                {
                    if ((vty.out_cursor -= w_size) > 0)
                    {
                        memmove(vty.out_buf, &(vty.out_buf[w_size]), vty.out_cursor);
                    }
                }
            }
        }
        usleep(10 * 1000);
    }*/
#ifdef _CEL_WIN
    cel_wsacleanup();
#endif

    return 0;
}
