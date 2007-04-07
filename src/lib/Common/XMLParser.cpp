/***************************************************************************
 *            XMLParser.cpp
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
 
#include "XMLParser.h"

using namespace std;

CXMLNode::CXMLNode(xmlNode* p_NodePtr, int p_nIdx)
{
  m_pNode = p_NodePtr;
  m_nIdx  = p_nIdx;
  m_nChildCount = 0;
}

CXMLNode::~CXMLNode()
{
  CXMLNode* pNode;
  m_NodeListIter = m_NodeList.begin();
  for(m_NodeListIter; m_NodeListIter != m_NodeList.end(); m_NodeListIter++) {
    pNode = (*m_NodeListIter).second;
    delete pNode;
  }
}

int CXMLNode::ChildCount()
{
  xmlNode* pTmpNode;
  
  if(m_nChildCount > 0) {
    return m_nChildCount;
  }
    
  for(pTmpNode = m_pNode->children; pTmpNode; pTmpNode = pTmpNode->next) {
    m_nChildCount++;
  }

  return m_nChildCount;
}

CXMLNode* CXMLNode::ChildNode(int p_nIdx)
{
  CXMLNode* pResult = NULL;
  int nIdx = 0;
  xmlNode* pTmpNode;
  
  m_NodeListIter = m_NodeList.find(p_nIdx);
  if(m_NodeListIter != m_NodeList.end()) {
    pResult = m_NodeList[p_nIdx];
    return pResult;
  }

  for(pTmpNode = m_pNode->children; pTmpNode; pTmpNode = pTmpNode->next) {
    
    if(nIdx == p_nIdx) {
      pResult = new CXMLNode(pTmpNode, nIdx);
      m_NodeList[nIdx] = pResult;
      break;
    }

    nIdx++;
  }
  return pResult;
}

CXMLNode* CXMLNode::FindNodeByName(std::string p_sName, 
                             bool p_bRecursive)
{
  CXMLNode* pResult = NULL;
  int i = 0;
                                 
  for(i = 0; i < ChildCount(); i++) {
    
    if(ChildNode(i)->Name().compare(p_sName) == 0) {
      pResult = ChildNode(i);
      break;
    }
      
    // recursively search children's child nodes
    if(p_bRecursive && (ChildNode(i)->ChildCount() > 0)) {
      pResult = ChildNode(i)->FindNodeByName(p_sName, p_bRecursive);
      if(pResult) {
        break;
      }
    }
      
  }
                                 
  return pResult;
}

CXMLNode* CXMLNode::FindNodeByValue(std::string p_sName, 
                              std::string p_sValue, 
                              bool p_bRecursive)
{
  CXMLNode* pResult = NULL;
  int i = 0;
                                 
  for(i = 0; i < ChildCount(); i++) {
    
    if((ChildNode(i)->Name().compare(p_sName) == 0) &&
       (ChildNode(i)->Value().compare(p_sValue) == 0)) {
      pResult = ChildNode(i);
      break;
    }
      
    // recursively search children's child nodes
    if(p_bRecursive && (ChildNode(i)->ChildCount() > 0)) {
      pResult = ChildNode(i)->FindNodeByValue(p_sName, p_sValue, p_bRecursive);
      if(pResult) {
        break;
      }
    }
      
  }
                                 
  return pResult;
}

std::string CXMLNode::Attribute(std::string p_sName)
{
  xmlAttr* attr = m_pNode->properties;
  while(attr) {
    string sAttr = (char*)attr->name;
    if(sAttr.compare(p_sName) == 0)    
      return (char*)attr->children->content;    
    
    attr = attr->next;
  } 
  return "";
}

unsigned int CXMLNode::AttributeAsUInt(std::string p_sName)
{
  string sAttribute = Attribute(p_sName);
  unsigned int nResult = 0;
  if(sAttribute.length() > 0) {
    nResult = strtoul(sAttribute.c_str(), NULL, 0);
  }
  return nResult;
}


std::string CXMLNode::Name()
{
  return (char*)m_pNode->name;
}

std::string CXMLNode::Value()
{
  if(m_pNode->children && m_pNode->children->content) {
    return (char*)m_pNode->children->content;
  }
  else {
    return "";
  }
}


CXMLDocument::~CXMLDocument()
{
  if(m_pRootNode != NULL) {
    delete m_pRootNode;
  }

  if(m_pDoc != NULL) {
    xmlCleanupParser();
    xmlFreeDoc(m_pDoc);
  }
}

bool CXMLDocument::Load(std::string p_sFileName)
{
  m_pDoc = xmlReadFile(p_sFileName.c_str(), "UTF-8", XML_PARSE_NOBLANKS);
  return (m_pDoc != NULL);
}

CXMLNode* CXMLDocument::RootNode()
{
  if(!m_pRootNode) {
    m_pRootNode = new CXMLNode(xmlDocGetRootElement(m_pDoc), 0);
  }
  return m_pRootNode;
}