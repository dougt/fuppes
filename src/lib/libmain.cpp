/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            libmain.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "../../include/fuppes.h"

#include <iostream>
#include "Common/Common.h"
#include "Common/Exception.h"
#include "SharedConfig.h"
#include "SharedLog.h"
#include "Fuppes.h"
#include "ContentDirectory/ContentDatabase.h"
#include "ContentDirectory/VirtualContainerMgr.h"
#include "DeviceSettings/DeviceIdentificationMgr.h"
#include "Plugins/Plugin.h"

#ifdef WIN32
#include <shellapi.h>
#include <windows.h>
#endif

#include <string.h>
#include <string>

using namespace std;

CFuppes* pFuppes = 0;

void printHelp()
{
	CSharedLog::Print("            FUPPES - %s", CSharedConfig::Shared()->GetAppVersion().c_str());
  CSharedLog::Print("    the Free UPnP Entertainment Service");
  CSharedLog::Print("      http://fuppes.ulrich-voelkel.de\n\n");
	
	CSharedLog::Print(" --friendly-name <name> set friendly name");
	CSharedLog::Print(" --log-level [0-3] set log level (0-none, 1-normal, 2-extended, 3-debug)");
	CSharedLog::Print(" --log-file <filename> set log file (default: none)");
										
#ifndef WIN32
	CSharedLog::Print(" --temp-dir <dir> set temp directory (default: /tmp/fuppes)");
	CSharedLog::Print(" --config-file <filename> use alternate config file (default ~/.fuppes/fuppes.cfg)");
	CSharedLog::Print(" --database-file <filename> use alternate database file (default ~/.fuppes/fuppes.db)");
	CSharedLog::Print(" --vfolder-config-file <filename> use alternate vfolder config file (default ~/.fuppes/vfolder.cfg)");
	CSharedLog::Print(" --plugin-dir <dir> directory containing fuppes plugins (default $prefix/lib/fuppes");
#else
	CSharedLog::Print(" --temp-dir <dir> set temp directory (default: %TEMP%/fuppes)");
	CSharedLog::Print(" --config-file <filename> use alternate config file (default %APPDATA%/FUPPES/fuppes.cfg)");
	CSharedLog::Print(" --database-file <filename> use alternate database file (default %APPDATA%/FUPPES/fuppes.db)");
	CSharedLog::Print(" --vfolder-config-file <filename> use alternate vfolder config file (default %APPDATA%/FUPPES/vfolder.cfg)");
#endif
}

int fuppes_init(int argc, char* argv[], void(*p_log_cb)(const char* sz_log))
{
	// already initialized
  if(pFuppes)
    return FUPPES_FALSE;    
  
  // setup winsockets
  #ifdef WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);
  #endif  
  
  // setup config
  CSharedLog::Shared()->SetCallback(p_log_cb);

  // arguments  
  string sConfigDir;
  string sDbFile;
  string sConfigFile;
  string sVFolderFile;
  string sFriendlyName;
  string sLogFileName;
	string sTempDir;
  int    nLogLevel = 1;
  string sPluginDir;
  
  #warning todo: check params
  
  for(int i = 0; i < argc; i++) {
		
		if(strcmp(argv[i], "--help") == 0) { 
			printHelp();
			return FUPPES_FALSE; 
		}		
    else if((strcmp(argv[i], "--config-dir") == 0) && (argc > i + 1)) {
      sConfigDir = argv[i + 1];
      if((sConfigDir.length() > 1) && (sConfigDir.substr(sConfigDir.length() - 1).compare(upnpPathDelim) != 0)) {
        sConfigDir += upnpPathDelim;
      }
    }
		else if((strcmp(argv[i], "--temp-dir") == 0) && (argc > i + 1)) {
      sTempDir = argv[i + 1];
    }
    else if((strcmp(argv[i], "--config-file") == 0) && (argc > i + 1)) {
      sConfigFile = argv[i + 1];
    }
    else if((strcmp(argv[i], "--database-file") == 0) && (argc > i + 1)) {
      sDbFile = argv[i + 1];
    }    
    else if((strcmp(argv[i], "--vfolder-config-file") == 0) && (argc > i + 1)) {
      sVFolderFile = argv[i + 1];
    }
    else if((strcmp(argv[i], "--friendly-name") == 0) && (argc > i + 1)) {
      sFriendlyName = argv[i + 1];
    }
    else if((strcmp(argv[i], "--log-level") == 0) && (argc > i + 1)) {
      nLogLevel = atoi(argv[i + 1]);
    }
    else if((strcmp(argv[i], "--log-file") == 0) && (argc > i + 1)) {
      sLogFileName = argv[i + 1];
    }
    
    else if((strcmp(argv[i], "--plugin-dir") == 0) && (argc > i + 1)) {
      sPluginDir = argv[i + 1];
    }
  }
  
  CSharedLog::SetLogFileName(sLogFileName);
  CSharedLog::Shared()->SetLogLevel(nLogLevel, false);
  
  CSharedConfig::Shared()->SetConfigDir(sConfigDir);
  CSharedConfig::Shared()->SetConfigFileName(sConfigFile);
  CSharedConfig::Shared()->SetDbFileName(sDbFile);
  CSharedConfig::Shared()->SetVFolderConfigFileName(sVFolderFile);  
  CSharedConfig::Shared()->FriendlyName(sFriendlyName);
	CSharedConfig::Shared()->TempDir(sTempDir);
	CSharedConfig::Shared()->pluginDir(sPluginDir);    

	xmlInitParser();

  CSharedLog::Print("            FUPPES - %s", CSharedConfig::Shared()->GetAppVersion().c_str());
  CSharedLog::Print("    the Free UPnP Entertainment Service");
  CSharedLog::Print("      http://fuppes.ulrich-voelkel.de\n");


  // init config
	#ifdef WIN32
	string appDir = argv[0];
	cout << appDir << endl;
	if(!CSharedConfig::Shared()->SetupConfig(appDir)) {
	#else
	if(!CSharedConfig::Shared()->SetupConfig()) {
	#endif
    return FUPPES_FALSE;
  }                 
                    

	
	if(!CDatabase::init("sqlite3")) {    
    CSharedLog::Print("failed to initialize sqlite database plugin.");
		return FUPPES_FALSE;
  }

  return FUPPES_TRUE;
}

void fuppes_set_error_callback(void(*p_err_cb)(const char* sz_err))
{
  CSharedLog::Shared()->SetErrorCallback(p_err_cb);
}

void fuppes_set_notify_callback(void(*p_notify_cb)(const char* sz_title, const char* sz_msg))
{
  CSharedLog::Shared()->SetNotifyCallback(p_notify_cb);
}

void fuppes_set_user_input_callback(void(*p_user_input_cb)(const char* sz_msg, char* sz_result, unsigned int n_buffer_size))
{
  CSharedLog::Shared()->SetUserInputCallback(p_user_input_cb);
}

int fuppes_start()
{
  if(pFuppes)
    return FUPPES_FALSE;

	#if !defined(WIN32) && !defined(MSG_NOSIGNAL) && !(SO_NOSIGPIPE)
	signal(SIGPIPE, SIG_IGN);
	#endif

  // create fuppes instance
  try {
    pFuppes = new CFuppes(CSharedConfig::Shared()->GetIPv4Address(), CSharedConfig::Shared()->GetUUID());
    CSharedConfig::Shared()->AddFuppesInstance(pFuppes);
    return FUPPES_TRUE;
  }
  catch(fuppes::Exception ex) {
    CSharedLog::Shared()->UserError(ex.what());    
    CSharedLog::Print("[exiting]");
    return FUPPES_FALSE;
  }
}

int fuppes_stop()
{
  if(!pFuppes)
    return FUPPES_FALSE;
    
  delete pFuppes;
  pFuppes = 0;
  return FUPPES_TRUE;
}

int fuppes_cleanup()
{
  xmlCleanupParser();
    
  // cleanup winsockets
  #ifdef WIN32
  WSACleanup();
  #endif

  return FUPPES_TRUE;
}

int fuppes_is_started()
{
  if(pFuppes != 0)
    return FUPPES_TRUE;
  else
    return FUPPES_FALSE;
}

const char* fuppes_get_version()
{
  return CSharedConfig::Shared()->GetAppVersion().c_str();
}

void fuppes_set_loglevel(int n_log_level)
{
  CSharedLog::Shared()->SetLogLevel(n_log_level, false);
}

void fuppes_inc_loglevel()
{
  CSharedLog::Shared()->ToggleLog();
}

void fuppes_rebuild_db()
{
  CContentDatabase::Shared()->RebuildDB();
}

void fuppes_update_db()
{
  CContentDatabase::Shared()->UpdateDB();
}

void fuppes_update_db_add_new()
{
  CContentDatabase::Shared()->AddNew();
}

void fuppes_update_db_remove_missing()
{
  CContentDatabase::Shared()->RemoveMissing();
}

void fuppes_rebuild_vcontainers()
{
  CVirtualContainerMgr::Shared()->RebuildContainerList();
}

void fuppes_get_http_server_address(char* sz_addr, int n_buff_size)
{
  stringstream sAddr;
  sAddr << "http://" << pFuppes->GetHTTPServerURL();
  strcpy(sz_addr, sAddr.str().c_str());
}

void fuppes_send_alive()
{
  pFuppes->GetSSDPCtrl()->send_alive();
}

void fuppes_send_byebye()
{
  pFuppes->GetSSDPCtrl()->send_byebye();
}

void fuppes_send_msearch()
{
  pFuppes->GetSSDPCtrl()->send_msearch();
}

void fuppes_print_info()
{
  cout << "general information:" << endl;
  cout << "  version     : " << CSharedConfig::Shared()->GetAppVersion() << endl;
  cout << "  hostname    : " << CSharedConfig::Shared()->GetHostname() << endl;
  cout << "  OS          : " << CSharedConfig::Shared()->GetOSName() << " " << CSharedConfig::Shared()->GetOSVersion() << endl;
  cout << "  build at    : " << __DATE__ << " " << __TIME__ << endl;
  cout << "  build with  : " << __VERSION__ << endl;
  cout << "  address     : " << CSharedConfig::Shared()->GetIPv4Address() << endl;
  //cout << "  sqlite      : " << CContentDatabase::Shared()->GetLibVersion() << endl;
  cout << "  log-level   : " << CSharedLog::Shared()->GetLogLevel() << endl;
  cout << "  webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
  cout << endl;
	cout << "configuration file:" << endl;
	cout << "  " << CSharedConfig::Shared()->GetConfigFileName() << endl;
	cout << endl;
	
  cout << CPluginMgr::printInfo().c_str() << endl;
  

#ifdef HAVE_ICONV
	cout << "iconv: enabled" << endl;
#else
	cout << "iconv: disabled" << endl;
#endif
	
#ifndef WIN32
#ifdef HAVE_INOTIFY
	cout << "inotify: enabled" << endl;
#else
	cout << "inotify: disabled" << endl;
#endif

#ifdef HAVE_UUID
	cout << "uuid: enabled" << endl;
#else
	cout << "uuid: disabled" << endl;
#endif
#endif
}

void fuppes_print_device_settings()
{
  CDeviceIdentificationMgr::Shared()->PrintSettings();
}
