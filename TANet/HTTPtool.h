#pragma once

#include <iostream>
#include <Windows.h>

VOID InitializeCurl();
VOID CleanupCurl();
BOOL PerformHttpRequest(const std::string& url, std::string& readBuffer, const std::string& postData = "");
BOOL SendLineNotifyMessage(const std::string& Message);