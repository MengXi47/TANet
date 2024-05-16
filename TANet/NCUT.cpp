#include "NCUT.h"
#include "HTTPtool.h"
#include <cstring>
#include <Windows.h>
#include <tchar.h>
#include <regex>

#ifdef _DEBUG
#define DEBUG(fmt, ...) (_tprintf(_T(fmt), __VA_ARGS__))
#else
#define DEBUG(...) (0)
#endif

namespace NCUT
{
    BOOL Network::ExtractIP(const std::string& buffer, std::string& ip)
    {
        std::regex urlPattern(R"(http://[\d\.]+:\d+/)");
        std::smatch matches;
        if (std::regex_search(buffer, matches, urlPattern)) {
            if (!matches.empty()) {
                ip = matches[0];
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL Network::ExtractToken(const std::string& buffer, std::string& token)
    {
        std::string startMarker = "window.location=\"";
        size_t startPos = buffer.find(startMarker);
        if (startPos != std::string::npos) {
            startPos += startMarker.length();
            size_t endPos = buffer.find("\";", startPos);
            if (endPos != std::string::npos) {
                std::string HTTPtoken = buffer.substr(startPos, endPos - startPos);
                size_t pos = HTTPtoken.find('?');
                if (pos != std::string::npos) {
                    token = HTTPtoken.substr(pos + 1);
                    return TRUE;
                }
            }
        }
        return FALSE;
    }

    Network::Network()
    {
        LoginData = nullptr;
        LogoutData = nullptr;
        InitializeCurl();
        return;
    }

    Network::~Network()
    {
        delete LoginData;
        delete LogoutData;
        CleanupCurl();
        return;
    }

    BOOL Network::Login()
    {
        delete LoginData;
        LoginData = nullptr;
        delete LogoutData;
        LogoutData = nullptr;

        LoginData = new Parameter();
        LogoutData = new Parameter();

        if (PerformHttpRequest("http://www.msftconnecttest.com/redirect", LoginData->Buffer) &&
            ExtractToken(LoginData->Buffer, LoginData->token) &&
            ExtractIP(LoginData->Buffer, LoginData->IP)) {
            // std::cout << LoginData->Buffer << std::endl;
            DEBUG("Obtaining URL and token successfully.\n");
        }
        else {
            DEBUG("Failed to obtain URL and token.\n");
            return FALSE;
        }

        std::string LoginUrl = LoginData->IP + "/fgtauth?" + LoginData->token;
        if (PerformHttpRequest(LoginUrl, LoginData->Buffer)) {
            DEBUG("URL and token authentication successful.\n");
        }
        else {
            DEBUG("URL and token authentication failed.\n");
            return FALSE;
        }

        std::string postData =
            "4Tredir=http%3A%2F%2Fwww.msftconnecttest.com%2Fredirect&magic=" +
            LoginData->token +
            "&username=dormroom%40ncut.edu.tw&password=ncut23924505";
        if (PerformHttpRequest(LoginData->IP, LogoutData->Buffer, postData) &&
            ExtractToken(LogoutData->Buffer, LogoutData->token) &&
            ExtractIP(LogoutData->Buffer, LogoutData->IP)) {
            DEBUG("Successfully logged into the Internet.\n");
        }
        else {
            DEBUG("Failed to log in to the network.\n");
            return FALSE;
        }

        delete LoginData;
        LoginData = nullptr;

        LogoutData->URL = LogoutData->IP + "logout?" + LogoutData->token;
        return TRUE;
    }

    BOOL Network::Logout()
    {
        std::string readBuffer;

        if (PerformHttpRequest(LogoutData->URL, readBuffer)) {
            DEBUG("Log out of the Internet successfully.\n");
        }
        else {
            DEBUG("Failed to log out of the Internet.\n");
            return FALSE;
        }

        delete LogoutData;
        LogoutData = nullptr;

        return TRUE;
    }

    std::string Network::GetLogoutURL() const
    {
        return LogoutData->URL;
    }
}