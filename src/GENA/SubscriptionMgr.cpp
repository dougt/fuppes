/***************************************************************************
 *            SubscriptionMgr.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "SubscriptionMgr.h"
#include "../RegEx.h"
#include "../UUID.h"

CSubscriptionMgr* CSubscriptionMgr::m_pInstance = 0;

CSubscriptionMgr* CSubscriptionMgr::Shared()
{
  if(m_pInstance == 0)
    m_pInstance = new CSubscriptionMgr();
  return m_pInstance;
}


bool CSubscriptionMgr::HandleSubscription(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  CSubscription* pSubscription = new CSubscription();
  if(!this->ParseSubscription(pRequest, pSubscription))
  {
    delete pSubscription;
    return false;
  }
  
  switch(pSubscription->GetType())
  {
    case ST_SUBSCRIBE:
      AddSubscription(pSubscription);
      pResponse->SetGENASubscriptionID(pSubscription->GetSID());
      break;
    case ST_RENEW:
      RenewSubscription(pSubscription);
      pResponse->SetGENASubscriptionID(pSubscription->GetSID());
      delete pSubscription;
      break;
    case ST_UNSUBSCRIBE:
      DeleteSubscription(pSubscription);
      delete pSubscription;
      break;
    
    default:
      delete pSubscription;
      return false;    
  }
  
  
  return true;
}


int CSubscriptionMgr::ParseSubscription(CHTTPMessage* pRequest, CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::ParseSubscription" << endl;
  cout << pRequest->GetHeader() << endl;
  cout << endl << "SUBSCR COUNT: " << m_Subscriptions.size() << endl;
  
  RegEx rxSubscribe("([SUBSCRIBE|UNSUBSCRIBE]+)", PCRE_CASELESS);
  if(!rxSubscribe.Search(pRequest->GetHeader().c_str()))
    return GENA_PARSE_ERROR;
  
  string sSID = "";
  RegEx rxSID("SID: *uuid:([A-Z|0-9|-]+)", PCRE_CASELESS);
  if(rxSID.Search(pRequest->GetHeader().c_str()))
    sSID = rxSID.Match(1);
  
  string sSubscribe = ToLower(rxSubscribe.Match(1));
  if(sSubscribe.compare("subscribe") == 0)
  {
    if(sSID.length() == 0)
    {
      pSubscription->SetType(ST_SUBSCRIBE);
      cout << "SUBSCRIPTION" << endl;
    }
    else
    {
      pSubscription->SetType(ST_RENEW);
      pSubscription->SetSID(sSID);
      cout << "RENEW" << endl;
    }
  }
  else if(sSubscribe.compare("unsubscribe") == 0)
  {
    pSubscription->SetType(ST_UNSUBSCRIBE);
    pSubscription->SetSID(sSID);
    cout << "UNSUBSCRIPTION" << endl;
  }
  
}

void CSubscriptionMgr::AddSubscription(CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::AddSubscription" << endl;
  
  string sSID = GenerateUUID();
  pSubscription->SetSID(sSID);
  
  m_Subscriptions[sSID] = pSubscription;
}

bool CSubscriptionMgr::RenewSubscription(CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::RenewSubscription" << endl;
  
  m_SubscriptionsIterator = m_Subscriptions.find(pSubscription->GetSID());
  if(m_SubscriptionsIterator != m_Subscriptions.end())
  {
    m_Subscriptions[pSubscription->GetSID()]->Renew();
  }
  else
  {
    return false;
  }
  
  return true;
}

bool CSubscriptionMgr::DeleteSubscription(CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::DeleteSubscription" << endl;
    
  m_SubscriptionsIterator = m_Subscriptions.find(pSubscription->GetSID());
  /* found device */
  if(m_SubscriptionsIterator != m_Subscriptions.end())
  { 
    m_Subscriptions.erase(pSubscription->GetSID());
  }
  else
  {
    return false;
  }
  
  return true;
}

/* SUBSCRIBE
  
SUBSCRIBE /UPnPServices/ContentDirectory/event/ HTTP/1.1
HOST: 192.168.0.3:48756
NT: upnp:event
TIMEOUT: Second-180
User-Agent: POSIX, UPnP/1.0, Intel MicroStack/1.0.1423
CALLBACK: http://192.168.0.23:54877/UPnPServices/ContentDirectory/event/
*/
  
/* RENEW

SUBSCRIBE /UPnPServices/ConnectionManager/event/ HTTP/1.1
HOST: 192.168.0.3:48756
SID: uuid:b7e3b9b4-7059-472e-95ba-5401708fa2de
TIMEOUT: Second-180
User-Agent: POSIX, UPnP/1.0, Intel MicroStack/1.0.1423
*/

/* UNSUBSCRIBE

UNSUBSCRIBE /UPnPServices/ContentDirectory/event/ HTTP/1.1
HOST: 192.168.0.3:58642
SID: uuid:b5117436-a4ef-4944-9fb0-30190a83e2aa
USER-AGENT: Linux/2.6.16.16, UPnP/1.0, Intel SDK for UPnP devices /1.2
*/