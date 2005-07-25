/***************************************************************************
 *            Common.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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

#ifndef _COMMON_H
#define _COMMON_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include <assert.h>

/*===============================================================================
 MACROS
===============================================================================*/

#define SAFE_DELETE(_x_) if(_x_){delete(_x_); _x_ = NULL;}

/*===============================================================================
 DEFINITIONS
===============================================================================*/

#define ASSERT assert

/*===============================================================================
 File Functions
===============================================================================*/

bool FileExists(std::string p_sFileName);
bool IsFile(std::string p_sFileName);
bool DirectoryExists(std::string p_sDirName);
bool IsDirectory(std::string p_sDirName);
std::string ExtractFileExt(std::string p_sFileName);

/*===============================================================================
 String Functions
===============================================================================*/

std::string ToLower(std::string p_sInput);
bool SplitURL(std::string p_sURL, std::string* p_sIPAddress, unsigned int* p_nPort);

/*===============================================================================
 WIN32 specific definitions
===============================================================================*/

#ifdef WIN32

/* T.S.NOTE: This must be defined to use InitializeCriticalSectionAndSpinCount() */
#define _WIN32_WINNT 0x0410 /* Windos 98 * or later */

#pragma comment(lib,"Wsock32.lib") 
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <shlwapi.h> /* For PathXXX functions */

/* Common */
#define upnpSleep               Sleep
#define upnpPathDelim           "\\"

/* Sockets */
#define upnpSocket              SOCKET
#define upnpSocketSuccess       0
#define upnpSocketClose         closesocket
#define upnpSocketFlag(_x_)     const char _x_[256] = ""

/* Threads */
#define fuppesThread                                        HANDLE
#define fuppesThreadStart(_handle_, _callback_)             _handle_ = CreateThread(NULL, 0, &_callback_, this, 0, NULL)
#define fuppesThreadClose(_handle_, _timeoutms_)             WaitForSingleObject(_handle_, _timeoutms_); CloseHandle(_handle_)
#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   _handle_ = CreateThread(NULL, 0, &_callback_, &_arg_, 0, NULL)
#define fuppesThreadCallback                                DWORD WINAPI

#define fuppesThreadMutex                                   CRITICAL_SECTION
#define fuppesThreadInitMutex(_mutex_)                      InitializeCriticalSectionAndSpinCount(_mutex_, 0x80000400)
#define fuppesThreadLockMutex(_mutex_)                      EnterCriticalSection(_mutex_)
#define fuppesThreadUnlockMutex(_mutex_)                    LeaveCriticalSection(_mutex_)

#else

/*===============================================================================
 NOT WIN32 specific definitions
===============================================================================*/

#include <pthread.h>

/* Common */
#define upnpSleep               usleep
#define upnpPathDelim           "/"

/* Sockets */
#define upnpSocket              int
#define upnpSocketSuccess       -1

#define upnpSocketClose         close
#define upnpSocketFlag(_x_)     int* _x_ = NULL

/* Threads */
#define fuppesThread                                        pthread_t
#define fuppesThreadStart(_handle_, _callback_)             pthread_create(&_handle_, NULL, &_callback_, this);
#define fuppesThreadClose(_handle_, _timeoutms_)            pthread_cancel(_handle_);
#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   pthread_create(&_handle_, NULL, &_callback_, &_arg_);
#define fuppesThreadCallback                                void*

#define fuppesThreadMutex                                   pthread_mutex_t
#define fuppesInitMutex(_mutex_)                            pthread_mutex_init(_mutex_, NULL)
#define fuppesLockMutex(_mutex_)                            pthread_mutex_lock(_mutex_)
#define fuppesUnlockMutex(_mutex_)                          pthread_mutex_unlock(_mutex_)

#endif

#endif /* _COMMON_H */
