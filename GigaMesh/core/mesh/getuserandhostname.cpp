//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

#include <GigaMesh/getuserandhostname.h>

#include <iostream>

#ifdef _MSC_VER // windows version for hostname and login
#include <winsock.h>
#include <WinBase.h>
#include <lmcons.h>
#elif defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#include <lmcons.h>
#else
#include <unistd.h> // gethostname, getlogin_r
#endif

//! System indipendent retrieval of username and hostname
//! as part of the technical meta-data
void getUserAndHostName(
        std::string& rUserName,
        std::string& rHostName
) {
#ifdef WIN32
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char hostname[256] = {0};
	auto error = gethostname(hostname, 256);

	if(error != 0)
	{
		std::cerr << "[GigaMesh] ERROR: Could not get hostname!" << std::endl;
	}
	char username[UNLEN- 1] = {0};
	DWORD len = UNLEN - 1;
	if(!GetUserNameA(username,&len))
	{
		std::cerr << "[GigaMesh] ERROR: Could not get username!" << std::endl;
	}
	WSACleanup();
#else
    #ifndef HOST_NAME_MAX
	    const size_t HOST_NAME_MAX = 256;
    #endif
    #ifndef LOGIN_NAME_MAX
		const size_t LOGIN_NAME_MAX = 256;
    #endif
	// Write hostname and username - see: https://stackoverflow.com/questions/27914311/get-computer-name-and-logged-user-name
	char hostname[HOST_NAME_MAX] = {0};
	char username[LOGIN_NAME_MAX] = {0};
	gethostname( hostname, HOST_NAME_MAX );
	auto error = getlogin_r( username, LOGIN_NAME_MAX );

	if(error != 0)
	{
		std::cerr << "[GigaMesh] ERROR: Could not get username!" << std::endl;
	}
#endif
	rUserName = username;
	rHostName = hostname;
}
