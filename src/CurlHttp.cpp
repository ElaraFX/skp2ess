#include "CurlHttp.h"
#include <iostream>



size_t writeFunc(void *ptr, size_t size, size_t membyte, void *stream)
{
	memset(stream, 0, size * membyte);
	memcpy(stream, ptr, size * membyte);
	printf((char*)(ptr));
	printf("\n");
	return size * membyte;
}

CurlHttp::CurlHttp()
{
	init();
}

CurlHttp::~CurlHttp()
{
	curl_global_cleanup();
}

void CurlHttp::init()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void CurlHttp::get(const char *url, const int port)
{
	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_PORT, port);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, retBuffer);

	int ret = curl_easy_perform(curl);
	
	if(ret != CURLE_OK)
	{
		printf("https get error!error code: %d, when get https url = %s, port = %d\n", ret, url, port);
	}

	curl_easy_cleanup(curl);
	curl = NULL;
}

void CurlHttp::post(const char *url, const int port, const char *postJsonStr)
{	
	CURL *curl = curl_easy_init();

	struct curl_slist *headSlist = NULL;
	headSlist = curl_slist_append(headSlist, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headSlist);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_PORT, port);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postJsonStr);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postJsonStr));
	curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, retBuffer);

	int ret = curl_easy_perform(curl);
	if(ret != CURLE_OK)
	{
		printf("https post error!error code: %d, when post https url = %s, port = %d, post data = %s\n", ret, url, port, postJsonStr);
	}

	curl_slist_free_all(headSlist);
	headSlist = NULL;
	curl_easy_cleanup(curl);
	curl = NULL;
}