#include "wintool.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Netlistmgr.h>
#include <wrl/client.h>

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define DEBUG(fmt, ...) (_tprintf(_T(fmt), __VA_ARGS__))
#else
#define DEBUG(...) (0)
#endif

namespace Network
{
    CheckNetwork_NLM::CheckNetwork_NLM()
    {
        hrCOM = CoInitialize(NULL);
    }

    CheckNetwork_NLM::~CheckNetwork_NLM()
    {
        CoUninitialize();
    }

    BOOL CheckNetwork_NLM::isConnect()
    {
        Microsoft::WRL::ComPtr<IUnknown> pUnknown;
        hrCOM = CoCreateInstance(
            CLSID_NetworkListManager,
            NULL, CLSCTX_ALL,
            IID_IUnknown,
            reinterpret_cast<void**>(pUnknown.GetAddressOf()));
        if (SUCCEEDED(hrCOM)) {
            Microsoft::WRL::ComPtr<INetworkListManager> pNetworkListManager;
            hrCOM = pUnknown.As(&pNetworkListManager);
            if (SUCCEEDED(hrCOM)) {
                VARIANT_BOOL isConnected = VARIANT_FALSE;
                hrCOM = pNetworkListManager->get_IsConnectedToInternet(&isConnected);

                if (SUCCEEDED(hrCOM)) {
                    return (isConnected == VARIANT_TRUE);
                }
            }
        }
        return false;
    }

    Sock::Sock()
    {
        WSADATA wsaData;
        int result;

        result = WSAStartup(MAKEWORD(2, 2), &wsaData);

        if (result != 0)
        {
            DEBUG("WSAStartup failed with error: %d\n", result);
            MessageBox(
                NULL,
                _T("WSAStartup failed."),
                _T("系統"),
                MB_TOPMOST | MB_ICONERROR);
        }

        if (LOBYTE(wsaData.wVersion) != 2 ||
            HIBYTE(wsaData.wVersion) != 2)
        {
            DEBUG("Could not find a usable version of Winsock.dll\n");
            WSACleanup();
            MessageBox(
                NULL,
                _T("Could not find a usable version of Winsock.dll"),
                _T("系統"),
                MB_TOPMOST | MB_ICONERROR);
        }

        DEBUG("Winsock 2.2 initialized\n");
    }

    Sock::~Sock()
    {
        WSACleanup();
    }

    BOOL Sock::GetLocalIP(CHAR* buf, INT buflen)
    {
        SOCKET ConnectSocket = INVALID_SOCKET;
        struct addrinfo* result = NULL,
            * ptr = NULL,
            hints, * res;
        BOOL iResult;
        INT status;
        CHAR hostname[128];
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        gethostname(hostname, sizeof(hostname));
        if ((status = getaddrinfo(
            hostname,
            NULL,
            &hints,
            &res)) != 0) {
            DEBUG("getaddrinfo: %s\n",
                gai_strerror(status));
            return FALSE;
        }
        iResult = FALSE;
        buf[0] = 0;
        for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
            VOID* addr;
            CHAR ipver[100];
            if (p->ai_family == AF_INET) {
                struct sockaddr_in* ipv4 =
                    (struct sockaddr_in*)p->ai_addr;
                addr = &(ipv4->sin_addr);
                strcpy_s(ipver, 100, "IPv4");
                inet_ntop(p->ai_family, addr, buf, buflen);
                iResult = TRUE;
            }
            else {
                struct sockaddr_in6* ipv6 =
                    (struct sockaddr_in6*)p->ai_addr;
                addr = &(ipv6->sin6_addr);
                strcpy_s(ipver, 100, "IPv6");
            }
        }
        freeaddrinfo(res);
        return iResult;
    }
}

BOOL CheckMutex(const LPCWSTR lpMutexName) // TRUE:Mutex存在 FALSE:Mutex不存在 
{
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, TRUE, lpMutexName);
    if (hMutex) {
        CloseHandle(hMutex); 
        return TRUE;
    }
    else {
        hMutex = CreateMutex(NULL, TRUE, lpMutexName);
        if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
            if (hMutex) {
                CloseHandle(hMutex);
            }
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
}

BOOL GetExePath(std::wstring& exePath)
{
    wchar_t pathBuffer[MAX_PATH];
    DWORD result = GetModuleFileNameW(NULL, pathBuffer, MAX_PATH);
    if (result == 0) {
        return FALSE;
    }
    else {
        exePath = std::wstring(pathBuffer);
        return TRUE;
    }
}

static std::wstring ExtractFileName(const std::wstring& path)
{
    size_t lastSlashPos = path.find_last_of(L"\\/");
    size_t lastDotPos = path.find_last_of(L'.');

    if (lastDotPos == std::wstring::npos || (lastSlashPos != std::wstring::npos && lastDotPos < lastSlashPos)) {
        lastDotPos = path.length();
    }

    if (lastSlashPos == std::wstring::npos) {
        lastSlashPos = 0;
    }
    else {
        lastSlashPos++;
    }

    return path.substr(lastSlashPos, lastDotPos - lastSlashPos);
}

BOOL AddExetoregRun()
{
    HKEY hKey;
    const wchar_t* regRunPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    LONG openRes = RegOpenKeyExW(HKEY_CURRENT_USER, regRunPath, 0, KEY_ALL_ACCESS, &hKey);

    if (openRes != ERROR_SUCCESS) {
        DEBUG("Unable to open registry key.");
        return FALSE;
    }

    std::wstring exePath;
    GetExePath(exePath);
    std::wstring FileName = ExtractFileName(exePath);
    LONG setRes = RegSetValueExW(
        hKey, FileName.c_str(), 
        0, 
        REG_SZ, 
        (const BYTE*)exePath.c_str(), 
        (exePath.size() + 1) * sizeof(wchar_t));

    if (setRes != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        DEBUG("Unable to set registry value.");
        return FALSE;
    }

    RegCloseKey(hKey);
    return TRUE;
}

BOOL GetWindowsUserName(std::string& username) 
{
    char buffer[MAX_PATH];
    DWORD bufferLength = MAX_PATH;

    if (GetUserNameA(buffer, &bufferLength)) {
        username = buffer;
        return TRUE;
    }
    else {
        DEBUG("Failed to get user name.");
        return FALSE;
    }
}
