#include "cel/net/httpfilter.h"

int cel_httpfilter_allowcors_init(CelHttpFilterAllowCors *cors,
                                  const char *allow_origins, 
                                  const char *allow_credentials,
                                  const char *allow_methods, 
                                  const char *allow_headers, 
                                  const char *expose_headers,
                                  long max_age)
{
    return 0;
}

void cel_httpfilter_allowcors_destroy(CelHttpFilterAllowCors *cors)
{
    ;
}
