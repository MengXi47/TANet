#pragma once

#include <iostream>
#include <Windows.h>

namespace Network
{
	class CheckNetwork_NLM
	{
	public:
		CheckNetwork_NLM();
		~CheckNetwork_NLM();
		BOOL isConnect();
	private:
		HRESULT hrCOM;
	};

	class Sock
	{
	public:
		Sock();
		~Sock();
		BOOL GetLocalIP(CHAR* buf, INT buflen);
	private:

	};
}

BOOL CheckMutex(const LPCWSTR lpMutexName);
BOOL GetExePath(std::wstring & exePath);
BOOL AddExetoregRun();
BOOL GetWindowsUserName(std::string & username);
