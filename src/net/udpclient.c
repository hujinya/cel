#include "cel/net/udpclient.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

int cel_udpclient_init(CelUdpClient *client, int addr_family)
{
    return cel_socket_init(&(client->sock), addr_family, SOCK_DGRAM, IPPROTO_UDP);
}

void cel_udpclient_destroy(CelUdpClient *client)
{
    cel_socket_destroy(&(client->sock));
}

CelUdpClient *cel_udpclient_new(int addr_family)
{
    CelUdpClient *client;

    if ((client = (CelUdpClient *)cel_malloc(sizeof(CelUdpClient))) != NULL)
    {
        if (cel_udpclient_init(client, addr_family) != -1)
            return client;
        cel_free(client);
    }

    return NULL;
}

void cel_udpclient_free(CelUdpClient *client)
{
    cel_udpclient_destroy(client);
    cel_free(client);
}
