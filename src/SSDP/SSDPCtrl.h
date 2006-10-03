/***************************************************************************
 *            SSDPCtrl.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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
 
#ifndef _SSDPCTRL_H
#define _SSDPCTRL_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "../Common.h"

#ifndef WIN32
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>
#include <list>

#include "UDPSocket.h"
#include "SSDPMessage.h"
#include "MSearchSession.h"
#include "NotifyMsgFactory.h"

/*===============================================================================
 CLASS ISSDPCtrl
===============================================================================*/

class ISSDPCtrl
{

/* <PUBLIC> */

public:
	  
  virtual void OnSSDPCtrlReceiveMsg(CSSDPMessage*) = 0;

/* <\PUBLIC> */

};

/*===============================================================================
 CLASS CSSDPCtrl
===============================================================================*/

class CSSDPCtrl: public IUDPSocket, IMSearchSession
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

		CSSDPCtrl(std::string p_sIPAddress, std::string p_sHTTPServerURL);
		virtual ~CSSDPCtrl();

/*===============================================================================
 INIT
===============================================================================*/

		void Start();
    void Stop();

/*===============================================================================
 GET
===============================================================================*/

		CUDPSocket* get_socket();

/*===============================================================================
 SEND
===============================================================================*/

		void send_msearch();
	  void send_alive();
	  void send_byebye();

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

	  void SetReceiveHandler(ISSDPCtrl* pHandler);
	  void OnUDPSocketReceive(CUDPSocket* pUDPSocket, CSSDPMessage* pSSDPMessage);
   	void OnSessionReceive(CMSearchSession* pSender, CSSDPMessage* pMessage);
	
    void OnSessionTimeOut(CMSearchSession* pSender);
  
/* <\PUBLIC> */

/* <PRIVATE> */

	private:
    
/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/
    void HandleMSearch(CSSDPMessage* pSSDPMessage);  
  
    void CleanupSessions();
  
/*===============================================================================
 MEMBERS
===============================================================================*/

    CUDPSocket         m_Listener;	
    CNotifyMsgFactory* m_pNotifyMsgFactory;
    fuppesThread       msearch_thread;
    sockaddr_in        m_LastMulticastEp;  
    std::string        m_sIPAddress;    
    std::string        m_sHTTPServerURL;
    ISSDPCtrl*         m_pReceiveHandler;
    fuppesThreadMutex  m_SessionReceiveMutex;
    fuppesThreadMutex  m_SessionTimedOutMutex;
  
    std::list<CMSearchSession*> m_SessionList;    
    std::list<CMSearchSession*>::iterator m_SessionListIterator;
    
    std::list<CHandleMSearchSession*> m_HandleMSearchThreadList;
    std::list<CHandleMSearchSession*>::iterator m_HandleMSearchThreadListIterator;

/* <\PRIVATE> */

};

#endif /* _SSDPCTRL_H */
