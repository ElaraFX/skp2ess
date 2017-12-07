#ifndef _CURL_HTTP_H_
#define _CURL_HTTP_H_

#include <curl.h>
#define MAX_RET_BUFFER 10000


class CurlHttp
{
public:
	char retBuffer[MAX_RET_BUFFER];

	CurlHttp();
	~CurlHttp();

	void init();	
	void get(const char *url, const int port);
	void post(const char *url, const int port, const char *postJsonStr);
};

#endif