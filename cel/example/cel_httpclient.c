#include "cel/net/httpwebrequest.h"

int httpclient_test(int argc, TCHAR *argv[])
{
	cel_cryptomutex_register(NULL, NULL);
	cel_ssllibrary_init();
	printf("OpenSSL version: %s\n", OPENSSL_VERSION_TEXT);
    printf("Run-time version: %s\n", OpenSSL_version(OPENSSL_VERSION));

	//char url[] = {"http://sfgit.gzsunrun.cn/oauth/token?grant_type=authorization_code"};
	char url[] = {"https://oauth.aliyun.com/v1/token"};
	long _response_code;
	char response[8192];
	size_t resp_size = 8192;
	char data[] = { "grant_type=authorization_code&code=sk8Zw0sP&redirect_uri=https://192.168.23.154/iam/auth/v3/login/aliyun&client_id=4241193690420488418&client_secret=9e6kfJoHVz4WYuB5J3HrD8jt6OSClcmPO8mvBVFxuROe6BFy26mXG6dtXRRyHvx4" };

	CelHttpWebRequest *client = cel_httpwebrequest_new(NULL);
	if (data != NULL) {
		cel_httprequest_set_method(&(client->req), CEL_HTTPM_POST);
	}
	cel_httprequest_set_url_str(&(client->req), url);
	if (data != NULL) {
		cel_httprequest_write(&(client->req), data, strlen(data));
	}
	cel_httprequest_end(&(client->req));
	if (cel_httpclient_execute(&(client->http_client), &(client->req), &(client->rsp)) != 0) {
		CEL_SETERR((CEL_ERR_USER, _T("url:%s,err_msg:%s"), 
			url, cel_geterrstr()));
		puts(cel_geterrstr());
		cel_httpwebrequest_free(client);
		return -1;
	}
	_response_code = (long)cel_httpresponse_get_statuscode(&(client->rsp));
	long size = cel_httpresponse_get_body_data(&(client->rsp), 0, 0, response, resp_size);
	response[size] = '\0'; 
	puts(response);
	cel_httpwebrequest_free(client);
	return 0;
}
