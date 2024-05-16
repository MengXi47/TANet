#pragma once

#include <Windows.h>
#include <iostream>



namespace NCUT
{
	class Network
	{
	public:
		Network();
		~Network();
		BOOL Login();
		BOOL Logout();
		std::string GetLogoutURL() const;
	private:
		struct Parameter
		{
			std::string Buffer;
			std::string URL;
			std::string IP;
			std::string token;
		};
		Parameter* LoginData;
		Parameter* LogoutData;
		BOOL ExtractIP(
			const std::string& buffer, 
			std::string& ip);
		BOOL ExtractToken(
			const std::string& buffer, 
			std::string& token);
	};

	typedef Network* PNetwork;
}
