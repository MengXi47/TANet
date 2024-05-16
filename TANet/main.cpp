#include <iostream>
#include <tchar.h>
#include <iomanip>
#include <sstream>
#include <Windows.h>
#include <process.h>
#include "NCUT.h"
#include "HTTPtool.h"
#include "wintool.h"

//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

#ifdef _DEBUG
#define DEBUG(fmt, ...) (_tprintf(_T(fmt), __VA_ARGS__))
#else
#define DEBUG(...) (0)
#endif

UINT WINAPI LoginNetworkThread(VOID*);
VOID ReportMessage();
std::string GetLocalTime();
static void LaunchIE(LPTSTR lpURL);

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);

    if (CheckMutex(_T("NCUT_TANetAutoLoginMutex")))
    {
        MessageBox(NULL, _T("程式已經開啟"), _T("勤益網路自動登入系統"), MB_TOPMOST | MB_ICONERROR);
        return 1;
    }

    HANDLE hThread{};
    while (TRUE)
    {
        hThread = (HANDLE)_beginthreadex(NULL, 0, LoginNetworkThread, NULL, 0, NULL);
        if (hThread)
        {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
    }

    std::cin.get();
    return 0;
}

UINT WINAPI LoginNetworkThread(VOID*)
{
    std::unique_ptr<NCUT::Network> YangHaoNetworkAuto = std::make_unique<NCUT::Network>();
    std::unique_ptr<Network::CheckNetwork_NLM> CheckNetwork = std::make_unique<Network::CheckNetwork_NLM>();
    while (TRUE)
    {
        if (!CheckNetwork->isConnect())
        {
            std::cout << GetLocalTime() << "\t偵測到網路斷線\n";

            if (YangHaoNetworkAuto->Login())
            {
                std::cout << GetLocalTime() << "\t登入成功\n"
                    << GetLocalTime() << "\t登出網址: "
                    << YangHaoNetworkAuto->GetLogoutURL()
                    << std::endl;
                Sleep(5000);
            }
        }
        Sleep(500);
    }
}

VOID ReportMessage()
{
    std::unique_ptr<Network::Sock> sock = std::make_unique<Network::Sock>();

    CHAR cIP[32]{};
    sock->GetLocalIP(cIP, sizeof(cIP));
    std::string strIP = cIP;

    std::string WindowsUserName;
    GetWindowsUserName(WindowsUserName);

    std::string MESSAGE =
        "\nIP: " + strIP +
        "\nUserName: " + WindowsUserName;

    SendLineNotifyMessage(MESSAGE);
}

static std::string GetLocalTime()
{
    time_t now = time(0);
    tm LocalTm;
    localtime_s(&LocalTm, &now);
    std::stringstream result;
    result << std::put_time(&LocalTm, "%Y/%m/%d %H:%M:%S");
    return result.str();
}

static void LaunchIE(LPTSTR lpURL)
{
    STARTUPINFO siStartupInfoApp;
    PROCESS_INFORMATION piProcessInfoApp;
    TCHAR acCommand[1024];
    _stprintf_s(acCommand,
        _T("\"C:\\Program Files\\Internet Explorer\\iexplore.exe\" \"%s\""),
        lpURL);
    ZeroMemory(&siStartupInfoApp, sizeof(siStartupInfoApp));
    ZeroMemory(&piProcessInfoApp, sizeof(piProcessInfoApp));
    siStartupInfoApp.cb = sizeof(siStartupInfoApp);
    if (!CreateProcess(
        NULL,
        acCommand,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &siStartupInfoApp,
        &piProcessInfoApp))
    {
        return;
    }
    WaitForSingleObject(piProcessInfoApp.hProcess, 0);
    CloseHandle(piProcessInfoApp.hProcess);
    CloseHandle(piProcessInfoApp.hThread);
}