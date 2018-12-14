/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#include "cel/net/httpfilter.h"

int cel_httpfilter_init(CelHttpFilter *filter)
{
    return 0;
}

void cel_httpfilter_destroy(CelHttpFilter *filter)
{
    ;
}

int cel_httpfilter_allowcors_hanlder(CelHttpFilter *filter, 
                                     CelHttpClient *client, 
                                     CelHttpRequest *req, CelHttpResponse *rsp)
{
    CelHttpFilterAllowCors *cors = (CelHttpFilterAllowCors *)filter;

//    // Allow default headers if nothing is specified.
//    if len(opts.AllowHeaders) == 0 {
//        opts.AllowHeaders = defaultAllowHeaders
//    }
//
//    for _, origin := range opts.AllowOrigins {
//pattern := regexp.QuoteMeta(origin)
//             pattern = strings.Replace(pattern, "\\*", ".*", -1)
//             pattern = strings.Replace(pattern, "\\?", ".", -1)
//             allowOriginPatterns = append(allowOriginPatterns, "^"+pattern+"$")
//    }
//
//    return func(ctx *context.Context) {
//        var (
//            origin           = ctx.Input.Header(headerOrigin)
//            requestedMethod  = ctx.Input.Header(headerRequestMethod)
//            requestedHeaders = ctx.Input.Header(headerRequestHeaders)
//            // additional headers to be added
//            // to the response.
//            headers map[string]string
//            )
//
//            if ctx.Input.Method() == "OPTIONS" &&
//                (requestedMethod != "" || requestedHeaders != "") {
//                    headers = opts.PreflightHeader(origin, requestedMethod, requestedHeaders)
//                        for key, value := range headers {
//                            ctx.Output.Header(key, value)
//                        }
//                        ctx.ResponseWriter.WriteHeader(http.StatusOK)
//                            return
//            }
//            headers = opts.Header(origin)
//
//                for key, value := range headers {
//                    ctx.Output.Header(key, value)
//                }
//    }
    return 0;
}

int cel_httpfilter_allowcors_init(CelHttpFilterAllowCors *cors,
                                  const char *allow_origins, 
                                  BOOL is_allow_credentials,
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
