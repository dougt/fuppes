/***************************************************************************
 *            SSDPCtrl.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
#ifndef _SSDPCTRL_H
#define _SSDPCTRL_H

#include "Common.h"

#ifndef WIN32
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>

#include "UDPSocket.h"
#include "SSDPMessage.h"
#include "SSDPSession.h"
#include "NotifyMsgFactory.h"

class ISSDPCtrl
{
  public:
	  virtual void OnSSDPCtrlReceiveMsg(CSSDPMessage*) = 0;
};

class CSSDPCtrl: public IUDPSocket, ISSDPSession
{
	public:
		CSSDPCtrl(std::string p_sIPAddress, std::string p_sHTTPServerURL);
		virtual ~CSSDPCtrl();
	
		void Start();
    void Stop();
	
		void send_msearch();
	  void send_alive();
	  void send_byebye();
	
		CUDPSocket* get_socket();
	
	  void SetReceiveHandler(ISSDPCtrl*);
	  void OnUDPSocketReceive(CUDPSocket*, CSSDPMessage*);
   	void OnSessionReceive(CSSDPSession* pSender, CSSDPMessage* pMessage);
	
	private:
		CUDPSocket *listener;	
    CNotifyMsgFactory* m_pNotifyMsgFactory;
		upnpThread  msearch_thread;
	  sockaddr_in last_multicast_ep;  
    std::string m_sIPAddress;    
  
  
		ISSDPCtrl* m_pReceiveHandler;
};

#endif /* _SSDPCTRL_H */
