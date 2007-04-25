/***************************************************************************
 *            UPnPSearch.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "UPnPSearch.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../ContentDirectory/ContentDatabase.h"
#include "../ContentDirectory/VirtualContainerMgr.h"

#include <sstream>
#include <iostream>

using namespace std;

CUPnPSearch::CUPnPSearch(std::string p_sMessage):
  CUPnPBrowseSearchBase(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_SEARCH, p_sMessage)
{
  m_sParentIds = "";
}                                     

CUPnPSearch::~CUPnPSearch()
{
}

unsigned int CUPnPSearch::GetContainerIdAsUInt()
{
  return HexToInt(m_sContainerID);
}


std::string BuildParentIdList(CContentDatabase* pDb, std::string p_sIds, std::string p_sDevice)
{
  stringstream sSql;
  sSql << 
    "select OBJECT_ID from MAP_OBJECTS " <<
    "where " <<
    "  PARENT_ID in (" << p_sIds << ") and " <<
    "  DEVICE " << p_sDevice;
  
  cout << sSql.str() << endl; fflush(stdout);
  
  pDb->ClearResult();
  pDb->Select(sSql.str());
  if(pDb->Eof()) {
    return "";
  }
  
  string sResult = "";
  while(!pDb->Eof()) {    
    sResult += pDb->GetResult()->GetValue("OBJECT_ID") + ", ";
    pDb->Next();
  }
  
  // remove trailing ", "
  if(sResult.length() > 2) {
    sResult = sResult.substr(0, sResult.length() - 2);
  }
  
  string sSub = BuildParentIdList(pDb, sResult, p_sDevice);
  if(sSub.length() > 0) {
    sResult = sResult + ", " + sSub + ", " + p_sIds;
  }
  
  return sResult;
}


std::string CUPnPSearch::BuildSQL(bool p_bCount)
{
  /*std::string sTest;
	sTest = "(upnp:class contains \"object.item.imageItem\") and (dc:title = \"test \\\"dahhummm\\\" [xyz] §$%&(abc) titel\") or author exists true and (title exists false and (author = \"test\" or author = \"dings\"))";
*/

  string sOpenBr;
	string sProp;
	string sOp;
	string sVal;
	string sCloseBr;
	string sLogOp = " and ";
	bool   bNumericProp = false;
	bool   bLikeOp  = false;
  bool   bBuildOK = false;
  bool   bVirtualSearch = false;
  
  stringstream sSql;

  
  string sDevice = " is NULL ";
  
  unsigned int nContainerId = GetContainerIdAsUInt();
  if(nContainerId > 0) {
    if(CVirtualContainerMgr::Shared()->IsVirtualContainer(nContainerId, GetDeviceSettings()->m_sVirtualFolderDevice)) {
       bVirtualSearch = true;
      sDevice = " = '" + GetDeviceSettings()->m_sVirtualFolderDevice + "' ";
    }
    
    if(m_sParentIds.length() == 0) {
      CContentDatabase* pDb = new CContentDatabase();       
      stringstream sIds;
      sIds << nContainerId;    
      m_sParentIds = BuildParentIdList(pDb, sIds.str(), sDevice);
      
      if(m_sParentIds.length() > 0) {
        m_sParentIds = m_sParentIds + ", " + sIds.str();
      }
      else {
        m_sParentIds = sIds.str();
      }
      delete pDb;
      
      cout << "PARENT ID LIST: " << m_sParentIds << endl; fflush(stdout);
    }
  }
  
  
  sSql <<
    "select ";
  if(p_bCount)
    sSql << " count(*) as COUNT ";
  else
    sSql << " * ";  
  
  sSql <<
    "from " <<
    "  OBJECTS o, MAP_OBJECTS m " <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.DEVICE " << sDevice << " and " <<
    "  m.DEVICE " << sDevice << " and " <<
    "  o.OBJECT_ID = m.OBJECT_ID ";  
  
  if(m_sParentIds.length() > 0) {
    sSql << " and " <<
      "  m.PARENT_ID in (" << m_sParentIds << ") ";        
  }
  

  // xbox 360 uses &quot; instead of "
	//if(GetDeviceSettings()->m_bXBox360Support) {
	  m_sSearchCriteria = StringReplace(m_sSearchCriteria, "&quot;", "\"");
	//}


  cout << m_sSearchCriteria << endl;

  RegEx rxSearch("(\\(*) *([\\w+:*\\w*]+) ([=|!=|<|<=|>|>=|contains|doesNotContain|derivedfrom|exists]+) (\".*?[^\\\\]\"|true|false) *(\\)*) *([and|or]*)");
	if(rxSearch.Search(m_sSearchCriteria.c_str())) {
	  do {
		  cout <<  rxSearch.Match(1) << " X " << rxSearch.Match(2) << " X " << rxSearch.Match(3) << " X " << rxSearch.Match(4) << " X " << rxSearch.Match(5) << " X " << rxSearch.Match(6) << endl;
		
		  // contains "and" on first loop
			// so we just append it when criterias can be found
		  sSql << " " << sLogOp << endl;
		
		  sOpenBr  = rxSearch.Match(1);
			sProp    = rxSearch.Match(2);
		  sOp      = rxSearch.Match(3);
			sVal     = rxSearch.Match(4);
			sCloseBr = rxSearch.Match(5);
			sLogOp   = rxSearch.Match(6);
			
			if(sOp.compare("exists") == 0) {
        bBuildOK = false;
			}
			else {
				
				
				bBuildOK = true;
				
				// replace property
				if(sProp.compare("upnp:class") == 0) {
				  sProp = "o.TYPE";
					bNumericProp = true;
				}
				else if(sProp.compare("dc:title") == 0) {
				  sProp = "o.TITLE";
					bNumericProp = false;
				}
        else if(sProp.compare("upnp:artist") == 0) {
				  sProp = "d.A_ARTIST";
					bNumericProp = false;
				}
        else if(sProp.compare("protocolInfo") == 0) {
				  sProp = "o.MIME_TYPE";
					bNumericProp = false;
				}        
				else {
				  bBuildOK = false;
				}
				
				
				// replace operator
				bLikeOp = false;
				if(sOp.compare("contains") == 0) {
				  if(bNumericProp)
					  sOp = "in";
					else
					  sOp = "like";
						
					bLikeOp = true;
				}
				else if(sOp.compare("derivedfrom") == 0) {
				  sOp = "in";
				}
        else if(sOp.compare("=") == 0) {          
        }
				else {
				  bBuildOK = false;
				}
				
				
				// trim value
				cout << "Val: " << sVal << " => ";
			  sVal = sVal.substr(1, sVal.length() - 2);
				cout << sVal << endl;
				
				// replace value
				if(sProp.compare("o.TYPE") == 0) { 
          sOp = "in";
          
				  if(sVal.compare("object.item.imageItem") == 0)
					  sVal = "(110, 111)";
					else if(sVal.compare("object.item.audioItem") == 0)
					  sVal = "(120, 121, 122)";	
					else if(sVal.compare("object.item.videoItem") == 0)
					  sVal = "(133, 131, 132)";
					else if(sVal.compare("object.container.person.musicArtist") == 0)
					  sVal = "(11)";          
					else if(sVal.compare("object.container.album.musicAlbum") == 0)
					  sVal = "(31)";
          else if(sVal.compare("object.container.genre.musicGenre") == 0)
            sVal = "(41)";
					else
					  bBuildOK = false;
				} 
				else if (!bNumericProp) {
				  if(bLikeOp)
				    sVal = "'%" + sVal + "%'";
					else
						sVal = "'" + sVal + "'";
				}
				
			} // != exists
		
		  if(bBuildOK)
  			sSql << sOpenBr << sProp << " " << sOp << " " << sVal << sCloseBr << " ";
			else
			  cout << "error parsing search request (part): " << rxSearch.Match(0) << endl;
			  
			
		}	while (rxSearch.SearchAgain());
	}
	else {
	  //cout << "no match" << endl;
	}

	
  
  // order by and limit are not needed
  // in a count request  
  if(!p_bCount) {  
    
    // order by
    #warning: todo 'sort'
    sSql << " order by o.TITLE ";
    
    
    // limit
	  if((m_nRequestedCount > 0) || (m_nStartingIndex > 0)) {
      sSql << " limit " << m_nStartingIndex << ", ";
      if(m_nRequestedCount == 0)
        sSql << "-1";
      else
        sSql << m_nRequestedCount;
    }
  }
	
  //cout << "SEARCH QUERY: " << endl << sSql.str() << endl << endl;	

  return sSql.str();
}



