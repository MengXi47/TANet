#define CURL_STATICLIB

#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include "curl/curl.h"

#ifdef _DEBUG
#pragma comment (lib, "libcurl_a_debug.lib")
#else
#pragma comment (lib, "libcurl_a.lib")
#endif

#ifdef _DEBUG
#define DEBUG(fmt, ...) (_tprintf(_T(fmt), __VA_ARGS__))
#else
#define DEBUG(...) (0)
#endif

#pragma comment (lib, "Ws2_32.lib") 
#pragma comment (lib, "Wldap32.lib") 
#pragma comment (lib, "Crypt32.lib")
#pragma comment (lib, "Normaliz.lib")

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

VOID InitializeCurl()
{
	curl_global_init(CURL_GLOBAL_ALL);
	return;
}

VOID CleanupCurl()
{
	curl_global_cleanup();
	return;
}

BOOL PerformHttpRequest(
	const std::string& url, 
	std::string& readBuffer, 
	const std::string& postData)
{
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        if (!postData.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        }
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            return FALSE;
        }
        curl_easy_cleanup(curl);
    }
    else {
        DEBUG("curl initialization failed.\n");
        return FALSE;
    }
    return TRUE;
}

BOOL SendLineNotifyMessage(const std::string& Message)
{
	CURL* curl;
	CURLcode res;
	struct curl_slist* headers = NULL;
	std::string readBuffer;
	std::string ResultMessage = "message=" + Message;
	curl = curl_easy_init();
	if (!curl) {
		// std::cout << "Curl initialization failed!" << std::endl;
		return FALSE;
	}
	curl_easy_setopt(curl, CURLOPT_URL, "https://notify-api.line.me/api/notify");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	headers = curl_slist_append(headers, "Authorization: Bearer kXwWB6eJApQCeWG09AiUhowgozRA49nI9xtCPy2F4Sw");
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ResultMessage.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	return res == CURLE_OK ? TRUE : FALSE;
}