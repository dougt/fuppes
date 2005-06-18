/***************************************************************************
 *            SharedConfig.cpp
 *
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <iostream>

#include "win32.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

//#include <string>
#include "SharedConfig.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN     64
#endif

//using namespace std;

shared_config* shared_config::instance = 0;

shared_config* shared_config::shared()
{
	if (instance == 0)
		instance = new shared_config();
	return instance;
}

shared_config::shared_config()
{
in_addr* addr;

#ifdef WIN32
  // Init windows sockets
  WSADATA wsaData;
  int nRet = WSAStartup(MAKEWORD(1, 1), &wsaData);
  if(0 == nRet)
  {
    // Get hostname
    char name[64] = "";
    nRet = gethostname(name, sizeof(name));
    if(0 == nRet)
    {
      hostname = name;

      // Get host
      struct hostent* host;
      host = gethostbyname(name);

      // Get host address
      //in_addr* addr;
      addr = (struct in_addr*)host->h_addr;
    }
    WSACleanup();
  }
    
#else

  char* name;
  gethostname(name, 64);	
  hostname = name;
	
  struct hostent* host;
  host = gethostbyname(name);
	
  //in_addr* addr;
  addr = (struct in_addr*)host->h_addr;
#endif

  ip = inet_ntoa(*addr);
}

string shared_config::get_app_name()
{
	return "fuppes";
}

string shared_config::get_app_fullname()
{
	return "Free UPnP Entertainment Service";
}

string shared_config::get_app_version()
{
	return "0.1";
}

string shared_config::get_hostname()
{
	return hostname;
}

string shared_config::get_ip()
{
	return ip;
}

string shared_config::get_udn()
{	
	return "12345678-aabb-0000-ccdd-1234eeff0000";
}
