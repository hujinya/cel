#include "cel/net/httpwebclient.h"
#include "cel/eventloop.h"
#include "cel/sys/envinfo.h"
#include "cel/multithread.h"
#include "cel/crypto/crypto.h"

#define MAX_FD 10240

static CelEventLoop evt_loop;
//static CelSslContext *sslctx;

static CelHttpWebContext httpweb_ctx;
static CelHttpWebClient *client;

#ifdef _CEL_UNIX
static CelSignals s_signals[] = 
{
    { SIGPIPE, "sigpipe", SIG_IGN }, 
    { 0, NULL, NULL }
};
static CelResourceLimits s_rlimits = { 0, MAX_FD, MAX_FD };
#endif

void httpwebclient_close(CelHttpWebClient *client, CelAsyncResult *result)
{
    _tprintf(_T("result = %ld\r\n"), result->ret);
    cel_httpwebclient_free(client);
}

static int httpwebclient_working(void *data)
{
    cel_eventloop_run(&evt_loop);
    /*_tprintf(_T("Event loop thread %d exit.(%s)"), 
       cel_thread_getid(), cel_geterrstr(cel_geterrno()));*/
    cel_thread_exit(0);

    return 0;
}

int httpwebclient_test(int argc, TCHAR *argv[])
{
    int i, td_num, work_num;
    void *ret_val;
    CelCpuMask mask;
    CelThread tds[CEL_THDNUM];

    cel_multithread_support();
    cel_cryptomutex_register(NULL, NULL);
    cel_ssllibrary_init();
#ifdef _CEL_UNIX
    cel_signals_init(s_signals);
    cel_resourcelimits_set(&s_rlimits);
#endif
    work_num = (td_num = (int)cel_getnprocs());
    if (work_num > CEL_THDNUM)
        work_num = work_num / 2;
    else if (work_num < 2)
        work_num = 2;
    work_num = 20;
    if (cel_eventloop_init(&evt_loop, td_num, 10240) == -1)
        return 1;
    for (i = 0; i < work_num; i++)
    {
        cel_setcpumask(&mask, i%td_num);
        if (cel_thread_create(&tds[i], NULL, &httpwebclient_working, NULL) != 0
            || cel_thread_setaffinity(&tds[i], &mask) != 0)
        {
            _tprintf(_T("Work thread create failed.(%s)\r\n"), cel_geterrstr(cel_geterrno()));
            return 1;
        }
    }

    //if (((sslctx = cel_sslcontext_new(
    //    cel_sslcontext_method(_T("SSLv23")))) == NULL
    //    || cel_sslcontext_set_own_cert(sslctx, "/etc/sunrun/vas_server.crt", 
    //    "/etc/sunrun/vas_server.key", _T("38288446")) == -1
    //    || cel_sslcontext_set_ciphersuites(
    //    sslctx, _T("AES:ALL:!aNULL:!eNULL:+RC4:@STRENGTH")) == -1))
    //{
    //    printf("Ssl context init failed.(%s)\r\n", cel_geterrstr(cel_geterrno()));
    //    return -1;
    //}
    memset(&httpweb_ctx, 0 ,sizeof(CelHttpWebContext));
    client = cel_httpwebclient_new(NULL, &httpweb_ctx);

    cel_httpwebclient_set_nonblock(client, 1);
    cel_eventloop_add_channel(&evt_loop, cel_httpwebclient_get_channel(client), NULL);

    cel_httprequest_set_method(&(client->req), CEL_HTTPM_POST);
    cel_httprequest_set_url_str(&(client->req), "https://192.168.1.230:8443/api/mn/session");
    cel_httprequest_set_header(&(client->req),
        CEL_HTTPHDR_CONTENT_TYPE, 
        "application/json", strlen("application/json"));
    cel_httprequest_printf(&(client->req), 
        "{\"username\":\"%s\",\"password\":\"%s\",\"action\":\"login\"}", 
        "hujinya", "password");
    cel_httprequest_end(&(client->req));
    puts((char *)client->req.s.buffer);
    cel_httpwebclient_async_execute_request(client, httpwebclient_close);

    for (i = 0; i < work_num; i++)
    {
        //_tprintf(_T("i %d \r\n"), i);
        cel_thread_join(tds[i], &ret_val);
    }

    return 0;
}
