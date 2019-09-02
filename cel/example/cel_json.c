#include "cel/json.h"
#include "cel/file.h"
#include "cel/error.h"

int json_test(int argc, TCHAR *argv[])
{
    size_t len;
    CHAR buf1[] = { "{ \"key0\\r\\n\\t\\b\\f\\/\\\\\\\"\" :[ {key" };
    CHAR buf2[] = { "001 : 1.3e+10 , \"key002\":[123, false,\"\\r\\n\\t\\b\\f\\/\\\\\\\"v"};
    CHAR buf3[] = {"alue0022\" , null] , },{\"key010\":{\"key0100\":\"value0100\"} }], \"key2\":[]}"};
    CHAR buf4[1024] = {'\0'};
    CelJson json;

    printf("%s%s%s\r\n", buf1, buf2, buf3);
    cel_json_init(&json);
    cel_json_deserialize_starts(&json);
    len = sizeof(buf1) - 1;
    cel_json_deserialize_update(&json, buf1, len);
    len = sizeof(buf2) -1 ;
    cel_json_deserialize_update(&json, buf2, len);
    len = sizeof(buf3) -1;
    cel_json_deserialize_update(&json, buf3, len);
    cel_json_deserialize_finish(&json);

    cel_json_serialize_starts(&json, 1);
    len = 1024;
    cel_json_serialize_update(&json, buf4, &len);
    cel_json_serialize_finish(&json);
    _putts(buf4);
    cel_json_destroy(&json);

    cel_json_init_file(&json, cel_fullpath_a("./profile.conf"));
    puts(cel_geterrstr_a());
    cel_json_serialize_starts(&json, 1);
    memset(buf4, 0, 1024);
    len = 128;
    cel_json_serialize_update(&json, buf4, &len);
    printf("%s", buf4);
    memset(buf4, 0, 1024);
    len = 256;
    cel_json_serialize_update(&json, buf4, &len);
    printf("%s", buf4);
    memset(buf4, 0, 1024);
    len = 512;
    cel_json_serialize_update(&json, buf4, &len);
    printf("%s", buf4);
    memset(buf4, 0, 1024);
    len = 1024;
    cel_json_serialize_update(&json, buf4, &len);
    printf("%s\r\n", buf4);
    cel_json_serialize_finish(&json);
    
    cel_json_destroy(&json);

    return 0;
}
