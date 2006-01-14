/***************************************************************************
 *            HTTPMessage.cpp
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "HTTPMessage.h"
#include "Common.h"
#include "SharedLog.h"

#include <iostream>
#include <sstream>

#ifndef DISABLE_TRANSCODING
#include "Transcoding/LameWrapper.h"
#include "Transcoding/WrapperBase.h"

  #ifndef DISABLE_VORBIS
  #include "Transcoding/VorbisWrapper.h"
  #endif
  
  #ifndef DISABLE_MUSEPACK
  #include "Transcoding/MpcWrapper.h"
  #endif

#endif

#include "RegEx.h"
#include "UPnPActionFactory.h"

/*===============================================================================
 CONSTANTS
===============================================================================*/

const std::string LOGNAME = "HTTPMessage";

/*===============================================================================
 CLASS CHTTPMessage
===============================================================================*/

/* <PUBLIC> */

fuppesThreadCallback TranscodeLoop(void *arg);
fuppesThreadMutex TranscodeMutex;

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CHTTPMessage::CHTTPMessage()
{
  /* Init */
  m_nHTTPVersion			  = HTTP_VERSION_UNKNOWN;
  m_nHTTPMessageType    = HTTP_MESSAGE_TYPE_UNKNOWN;
	m_sHTTPContentType    = "";
  m_nBinContentLength   = 0; 
  m_nBinContentPosition = 0;
  m_nContentLength      = 0;
  m_pszBinContent       = NULL;
  m_bIsChunked          = false;
  m_TranscodeThread     = (fuppesThread)NULL;
  m_pUPnPItem           = NULL;
  m_bIsBinary           = false;
}

CHTTPMessage::~CHTTPMessage()
{
  /* Cleanup */
  if(m_pszBinContent)
    delete[] m_pszBinContent;
    
  if(m_TranscodeThread)
    fuppesThreadClose(m_TranscodeThread);

  if(m_fsFile.is_open())
    m_fsFile.close();
}

/*===============================================================================
 INIT
===============================================================================*/

void CHTTPMessage::SetMessage(HTTP_MESSAGE_TYPE nMsgType, std::string p_sContentType)
{
	CMessageBase::SetMessage("");
  
  m_nHTTPMessageType  = nMsgType;
	m_sHTTPContentType  = p_sContentType;
  m_nBinContentLength = 0;
}

bool CHTTPMessage::SetMessage(std::string p_sMessage)
{
  /*cout << "SET MESSAGE" << endl;
  fflush(stdout);*/
     
  CMessageBase::SetMessage(p_sMessage);  
  CSharedLog::Shared()->DebugLog(LOGNAME, p_sMessage);  

  /*cout << "IN SET MESSAGE" << endl;
  fflush(stdout);*/
  
  return BuildFromString(p_sMessage);
}

void CHTTPMessage::SetBinContent(char* p_szBinContent, unsigned int p_nBinContenLength)
{ 
  m_bIsBinary = true;

  m_nBinContentLength = p_nBinContenLength;      
  m_pszBinContent     = new char[m_nBinContentLength + 1];    
  memcpy(m_pszBinContent, p_szBinContent, m_nBinContentLength);
  m_pszBinContent[m_nBinContentLength] = '\0';   
}

/*===============================================================================
 GET MESSAGE DATA
===============================================================================*/

bool CHTTPMessage::GetAction(CUPnPBrowse* pBrowse)
{
  BOOL_CHK_RET_POINTER(pBrowse);

  /* Build UPnPAction */
  CUPnPActionFactory ActionFactory;
  return ActionFactory.BuildActionFromString(m_sContent, pBrowse);
}

std::string CHTTPMessage::GetHeaderAsString()
{
	stringstream sResult;
	string sVersion;
	string sType;
	string sContentType;
	
  /* Version */
	switch(m_nHTTPVersion)
	{
		case HTTP_VERSION_1_0: sVersion = "HTTP/1.0"; break;
		case HTTP_VERSION_1_1: sVersion = "HTTP/1.1"; break;
    default:               ASSERT(0);             break;
  }
	
  /* Message Type */
	switch(m_nHTTPMessageType)
	{
		case HTTP_MESSAGE_TYPE_GET:	          /* todo */			                            break;
    case HTTP_MESSAGE_TYPE_HEAD:	        /* todo */			                            break;
		case HTTP_MESSAGE_TYPE_POST:          /* todo */		                              break;
		case HTTP_MESSAGE_TYPE_200_OK:        sResult << sVersion << " " << "200 OK\r\n"; break;
	  case HTTP_MESSAGE_TYPE_404_NOT_FOUND:	/* todo */			                            break;
    default:                              ASSERT(0);                                  break;
	}
	
  /* Content length */
  if(!m_bIsBinary)
  {
    sResult << "CONTENT-LENGTH: " << (int)strlen(m_sContent.c_str()) << "\r\n";
  }
  else
  {
    if(!m_bIsTranscoding && (m_nBinContentLength > 0))
      sResult << "CONTENT-LENGTH: " << m_nBinContentLength << "\r\n";
    else
      sResult << "CONTENT-LENGTH: 0\r\n";
  }      

	
  /* Content type */
	/*switch(m_HTTPContentType)
	{
		case HTTP_CONTENT_TYPE_TEXT_HTML:  sContentType = "text/html";  break;
		case HTTP_CONTENT_TYPE_TEXT_XML:   sContentType = "text/xml; charset=\"utf-8\"";   break;
		case HTTP_CONTENT_TYPE_AUDIO_MPEG: sContentType = "audio/mpeg"; break;
    case HTTP_CONTENT_TYPE_IMAGE_PNG : sContentType = "image/png";  break;      
    default:                           ASSERT(0);                   break;	
	}*/

	sResult << "CONTENT-TYPE: " << m_sHTTPContentType << "\r\n";	
	sResult << "SERVER: Linux/2.6.x, UPnP/1.0, fuppes/0.3 \r\n";
	sResult << "DATE: Sat, 12 Nov 2005 18:36:58 GMT\r\n";
  sResult << "CONNECTION: close\r\n";
  sResult << "ACCEPT-RANGES: bytes\r\n";
  
  //sResult << "contentFeatures.dlna.org: \r\n";
  
	sResult << "EXT: \r\n";
	
  /* Transfer-Encoding */
  if(m_nHTTPVersion == HTTP_VERSION_1_1)
  {
    /*if(m_bIsChunked)
    {
      sResult << "TRANSFER-ENCODING: chunked\r\n";
    }*/
  }	
	
	sResult << "\r\n";
	return sResult.str();
}

std::string CHTTPMessage::GetMessageAsString()
{
  stringstream sResult;
  sResult << GetHeaderAsString();
  sResult << m_sContent;
  return sResult.str();
}

unsigned int CHTTPMessage::GetBinContentChunk(char* p_sContentChunk, unsigned int p_nSize, unsigned int p_nOffset)
{
  /* read from file */
  if(m_fsFile.is_open())
  {
    //cout << "read from file" << endl;
    unsigned int nRest = m_nBinContentLength - m_nBinContentPosition;    
    if(nRest >= p_nSize)
    {
      m_fsFile.read(p_sContentChunk, p_nSize);      
      m_nBinContentPosition += p_nSize;
      return p_nSize;
    }
    else
    {
      m_fsFile.read(p_sContentChunk, nRest);
      m_nBinContentPosition += nRest;
      return nRest;
    }
  }  
  
  /* read transcoded data from memory */
  else
  {    
    fuppesThreadLockMutex(&TranscodeMutex);      
    
    unsigned int nRest       = m_nBinContentLength - m_nBinContentPosition;
    unsigned int nDelayCount = 0;  
    while(m_bIsTranscoding && (nRest < p_nSize) && !m_bBreakTranscoding)
    { 
      nRest = m_nBinContentLength - m_nBinContentPosition;

      stringstream sLog;
      sLog << "we are sending faster then we can transcode!" << endl;
      sLog << "  Try     : " << (nDelayCount + 1) << endl;
      sLog << "  Lenght  : " << m_nBinContentLength << endl;
      sLog << "  Position: " << m_nBinContentPosition << endl;
      sLog << "  Size    : " << p_nSize << endl;
      sLog << "  Rest    : " << nRest << endl;
      sLog << "delaying send-process!";

      CSharedLog::Shared()->Critical(LOGNAME, sLog.str());    
      fuppesThreadUnlockMutex(&TranscodeMutex);
      fuppesSleep(100);
      fuppesThreadLockMutex(&TranscodeMutex);    
      nDelayCount++;
      
      /* if bufer is still empty after n tries
         the machine seems to be too slow. so
         we give up */
      if (nDelayCount == 100) /* 100 * 100ms = 10sec */
      {
        fuppesThreadLockMutex(&TranscodeMutex);    
        m_bBreakTranscoding = true;
        fuppesThreadUnlockMutex(&TranscodeMutex);
        return 0;
      }
    }
    
    if(nRest > p_nSize)
    {
      memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], p_nSize);    
      m_nBinContentPosition += p_nSize;
      fuppesThreadUnlockMutex(&TranscodeMutex);
      return p_nSize;
    }
    else if((nRest < p_nSize) && !m_bIsTranscoding)
    {
      memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], nRest);
      m_nBinContentPosition += nRest;
      fuppesThreadUnlockMutex(&TranscodeMutex);
      return nRest;
    }
     
    fuppesThreadUnlockMutex(&TranscodeMutex);
  
  }
  return 0;
}

/*===============================================================================
 OTHER
===============================================================================*/

bool CHTTPMessage::BuildFromString(std::string p_sMessage)
{
  m_nBinContentLength = 0;
  m_sMessage = p_sMessage;
  m_sContent = p_sMessage;  
  
  bool bResult = false;

  /*cout << "BUILD FROM STR" << endl;
  fflush(stdout);*/

  /* Message GET */
  RegEx rxGET("GET +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxGET.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_GET;

    string sVersion = rxGET.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxGET.Match(1);			
    bResult = true;
  }

  /* Message HEAD */
  RegEx rxHEAD("HEAD +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxHEAD.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_HEAD;

    string sVersion = rxHEAD.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxHEAD.Match(1);			
    bResult = true;
  }
  
  /* Message POST */
  RegEx rxPOST("POST +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxPOST.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_POST;

    string sVersion = rxPOST.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxPOST.Match(1);			

    bResult = ParsePOSTMessage(p_sMessage);
  }
  
  /*cout << "END BUILD FROM STR" << endl;
  fflush(stdout);  */
  
  return bResult;
}

bool CHTTPMessage::LoadContentFromFile(std::string p_sFileName)
{
  m_bIsChunked = true;
  m_bIsBinary  = true;
  
  //fstream fFile;    
  m_fsFile.open(p_sFileName.c_str(), ios::binary|ios::in);
  if(m_fsFile.fail() != 1)
  { 
    m_fsFile.seekg(0, ios::end); 
    m_nBinContentLength = streamoff(m_fsFile.tellg()); 
    m_fsFile.seekg(0, ios::beg);
    //fFile.close();
  } 
	
  return true;
}


bool CHTTPMessage::TranscodeContentFromFile(std::string p_sFileName)
{ 
  m_bIsChunked = true;
  m_bIsBinary  = true;
  //m_nHTTPVersion = HTTP_VERSION_1_1;  
  
  m_bBreakTranscoding = false;
  
  m_TranscodeThread = (fuppesThread)NULL;
  fuppesThreadInitMutex(&TranscodeMutex);
      
  CTranscodeSessionInfo* session = new CTranscodeSessionInfo();
  session->m_pHTTPMessage = this;
  session->m_sFileName    = p_sFileName;
  
  fuppesThreadStartArg(m_TranscodeThread, TranscodeLoop, *session); 
  m_bIsTranscoding = true;
  
  fuppesSleep(1000); /* let the encoder work for a second */
  
  return true;
}

fuppesThreadCallback TranscodeLoop(void *arg)
{
  #ifndef DISABLE_TRANSCODING
  CTranscodeSessionInfo* pSession = (CTranscodeSessionInfo*)arg;
	  
  /* init lame encoder */
  CLameWrapper* pLameWrapper = new CLameWrapper();
  if(!pLameWrapper->LoadLib())
  {    
    delete pLameWrapper;
    delete pSession; 
    pSession->m_pHTTPMessage->m_bIsTranscoding = false;     
    fuppesThreadExit();
  }
  pLameWrapper->SetBitrate(LAME_BITRATE_320);
  pLameWrapper->Init();	

  string sExt = ExtractFileExt(pSession->m_sFileName);

  CDecoderBase* pDecoder = NULL;
  if(ToLower(sExt).compare("ogg") == 0)
  {
    #ifndef DISABLE_VORBIS
    pDecoder = new CVorbisDecoder();
    #endif
  }
  else if(ToLower(sExt).compare("mpc") == 0)
  {
    #ifndef DISABLE_MUSEPACK
    pDecoder = new CMpcDecoder();
    #endif
  }

  /* init decoder */  
  if(!pDecoder || !pDecoder->LoadLib() || !pDecoder->OpenFile(pSession->m_sFileName))
  {
    delete pDecoder;
    delete pLameWrapper;
    delete pSession;
    pSession->m_pHTTPMessage->m_bIsTranscoding = false;
    fuppesThreadExit();
  }
   
  /* begin transcode */
  long  samplesRead  = 0;    
  int   nLameRet     = 0;
  int   nAppendCount = 0;
  int   nAppendSize  = 0;
	char* sAppendBuf   = NULL; 
  #ifndef DISABLE_MUSEPACK
  int nBufferLength = 0;  
  short int* pcmout;
  CMpcDecoder* pTmp = dynamic_cast<CMpcDecoder*>(pDecoder);  
  if (pTmp)
  {
	  pcmout = new short int[MPC_DECODER_BUFFER_LENGTH * 4];
    nBufferLength = MPC_DECODER_BUFFER_LENGTH * 4;
  }
  else
  {
    pcmout = new short int[4096];    
    nBufferLength = 4096;
  }  
  #else
  short int* pcmout = new short int[4096];
  int nBufferLength = 4096;
  #endif
  
  stringstream sLog;
  sLog << "start transcoding \"" << pSession->m_sFileName << "\"";
  CSharedLog::Shared()->Log(LOGNAME, sLog.str());
    
  while(((samplesRead = pDecoder->DecodeInterleaved((char*)pcmout, nBufferLength)) >= 0) && !pSession->m_pHTTPMessage->m_bBreakTranscoding)
  {
    /* encode */
    nLameRet = pLameWrapper->EncodeInterleaved(pcmout, samplesRead);      
  
    /* append encoded mp3 frames to the append buffer */
    char* tmpBuf = NULL;
    if(sAppendBuf != NULL)
    {
      tmpBuf = new char[nAppendSize];
      memcpy(tmpBuf, sAppendBuf, nAppendSize);
      delete[] sAppendBuf;      
    }
    
    sAppendBuf = new char[nAppendSize + nLameRet];
    if(tmpBuf != NULL)
    {
      memcpy(sAppendBuf, tmpBuf, nAppendSize);
      delete[] tmpBuf;
    }
    memcpy(&sAppendBuf[nAppendSize], pLameWrapper->GetMp3Buffer(), nLameRet);
    nAppendSize += nLameRet;
    nAppendCount++;
    /* end append */
    
    
    if(nAppendCount == 100)
    {      
      /* append encoded audio to bin content buffer */
      fuppesThreadLockMutex(&TranscodeMutex);
      
      /* merge existing bin content and new buffer */
      char* tmpBuf = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nAppendSize];
      memcpy(tmpBuf, pSession->m_pHTTPMessage->m_pszBinContent, pSession->m_pHTTPMessage->m_nBinContentLength);
      memcpy(&tmpBuf[pSession->m_pHTTPMessage->m_nBinContentLength], sAppendBuf, nAppendSize);
      
      /* recreate bin content buffer */
      delete[] pSession->m_pHTTPMessage->m_pszBinContent;
      pSession->m_pHTTPMessage->m_pszBinContent = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nAppendSize];
      memcpy(pSession->m_pHTTPMessage->m_pszBinContent, tmpBuf, pSession->m_pHTTPMessage->m_nBinContentLength + nAppendSize);
            
      /* set the new content length */
      pSession->m_pHTTPMessage->m_nBinContentLength += nAppendSize;
      
      delete[] tmpBuf;
      
      /* reset append buffer an variables */
      nAppendCount = 0;
      nAppendSize  = 0;
      delete[] sAppendBuf;
      sAppendBuf = NULL;
      
      fuppesThreadUnlockMutex(&TranscodeMutex);  
      /* end append */      
    }
    
  } /* while */  
  
  
  /* delete buffers */
  if(pcmout)
    delete[] pcmout;
	
  if(sAppendBuf)
    delete[] sAppendBuf;
  
  
  /* flush and end transcoding */
  if(!pSession->m_pHTTPMessage->m_bBreakTranscoding)
  {
    /*sLog.str("");
    sLog << "done transcoding loop now flushing" << endl;
    CSharedLog::Shared()->Log(LOGNAME, sLog.str()); */
  
    /* flush mp3 */
    nLameRet = pLameWrapper->Flush();
  
    /* append encoded audio to bin content buffer */
    fuppesThreadLockMutex(&TranscodeMutex);
      
    char* tmpBuf = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nLameRet];
    if(pSession->m_pHTTPMessage->m_nBinContentLength > 0)
      memcpy(tmpBuf, pSession->m_pHTTPMessage->m_pszBinContent, pSession->m_pHTTPMessage->m_nBinContentLength);
    memcpy(&tmpBuf[pSession->m_pHTTPMessage->m_nBinContentLength], pLameWrapper->GetMp3Buffer(), nLameRet);
    
    delete[] pSession->m_pHTTPMessage->m_pszBinContent;
    pSession->m_pHTTPMessage->m_pszBinContent = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nLameRet];
    memcpy(pSession->m_pHTTPMessage->m_pszBinContent, tmpBuf, pSession->m_pHTTPMessage->m_nBinContentLength + nLameRet);
       
    pSession->m_pHTTPMessage->m_nBinContentLength += nLameRet;
    delete[] tmpBuf;
    pSession->m_pHTTPMessage->m_bIsTranscoding = false;    
  
    fuppesThreadUnlockMutex(&TranscodeMutex);  
    /* end append */      
    
    sLog.str("");
    sLog << "done transcoding \"" << pSession->m_sFileName << "\"";
    CSharedLog::Shared()->Log(LOGNAME, sLog.str());  
  }
  /* break transcoding */
  else
  {
    cout << "break transcoding" << endl;
    fflush(stdout);
  }    
  /* end transcode */
  
  
  /* cleanup and exit */
  pDecoder->CloseFile();  
  delete pDecoder;
  delete pLameWrapper;
  delete pSession;  
  fuppesThreadExit();
  
  #endif /* ifndef DISABLE_TRANSCODING */
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

bool CHTTPMessage::ParsePOSTMessage(std::string p_sMessage)
{
  /*POST /UPnPServices/ContentDirectory/control HTTP/1.1
    Host: 192.168.0.3:32771
    SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#Browse"
    CONTENT-TYPE: text/xml ; charset="utf-8"
    Content-Length: 467*/
  
  RegEx rxSOAP("SOAPACTION: *\"(.+)\"", PCRE_CASELESS);
	if(rxSOAP.Search(p_sMessage.c_str()))
	{
    string sSOAP = rxSOAP.Match(1);
    //cout << "[HTTPMessage] SOAPACTION " << sSOAP << endl;
	}
      
  /* Content length */
  RegEx rxContentLength("CONTENT-LENGTH: *(\\d+)", PCRE_CASELESS);
  if(rxContentLength.Search(p_sMessage.c_str()))
  {
    string sContentLength = rxContentLength.Match(1);    
    m_nContentLength = std::atoi(sContentLength.c_str());
  }
  
  if((unsigned int)m_nContentLength >= p_sMessage.length())                      
    return false;
  
  m_sContent = p_sMessage.substr(p_sMessage.length() - m_nContentLength, m_nContentLength);
  
  return true;
}

/* <\PRIVATE> */
