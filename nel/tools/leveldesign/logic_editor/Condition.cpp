// Condition.cpp: implementation of the CCondition class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "Condition.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CConditionNode implementation
//////////////////////////////////////////////////////////////////////

CConditionNode::CConditionNode(const CConditionNode &node)
{
	this->m_dComparand = node.m_dComparand;
	this->m_pParentNode = node.m_pParentNode;
	this->m_sConditionName = node.m_sConditionName;
	this->m_sOperator = node.m_sOperator;
	this->m_sVariableName = node.m_sVariableName;
	this->m_type = node.m_type;

	// copy sub condition node tree
	CConditionNode *pNode, *newNode;
	POSITION pos = node.m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			newNode = new CConditionNode( *pNode );
			this->m_ctSubTree.AddTail( newNode );
			newNode->m_pParentNode = this;
		}
	}

}



CConditionNode::~CConditionNode()
{
	CConditionNode *pNode = NULL;

	// delete all sub nodes
	POSITION pos = m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			delete pNode;
			pNode = NULL;
		}
	}
}

const CString & CConditionNode::getNodeAsString() const
{
	m_sNodeString.Empty();

	if (m_type == NOT)
		m_sNodeString = "NOT";
	else if (m_type == TERMINATOR)
		m_sNodeString = "term";
	else if (m_type == SUB_CONDITION)
		m_sNodeString = m_sConditionName;
	else // comparison
	{
		m_sNodeString.Format("%s %s %g",LPCTSTR(m_sVariableName),LPCTSTR(m_sOperator), m_dComparand );
	}

	return m_sNodeString;
}



void CConditionNode::changeConditionName( const CString &old, const CString &newName)
{
	CConditionNode *pNode = NULL;

	POSITION pos = m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			pNode->changeConditionName(old, newName);
		}
	}

	//
	if ( m_sConditionName == old)
		m_sConditionName = newName;
}

void CConditionNode::conditionDeleted( const CString &name)
{
	CConditionNode *pNode = NULL;

	POSITION oldpos;
	POSITION pos = m_ctSubTree.GetHeadPosition();
	while (pos != NULL)
	{
		oldpos = pos;
		pNode = m_ctSubTree.GetNext( pos );

		if (pNode != NULL)
		{
			if ( pNode->m_sConditionName != name)
				pNode->conditionDeleted(name);
			else
			{
				this->m_ctSubTree.RemoveAt( oldpos );
				delete pNode;
				pNode = NULL;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CCondition implementation
//////////////////////////////////////////////////////////////////////
CCondition::CCondition()
{
}


CCondition::CCondition( const CCondition &cond)
{
	this->m_sName = cond.m_sName;

	// copy sub condition node tree
	CConditionNode *pNode, *newNode;
	POSITION pos = cond.m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			newNode = new CConditionNode( *pNode );
			this->m_ctConditionTree.AddTail( newNode );
		}
	}	

}


CCondition::~CCondition()
{
	CConditionNode *pNode = NULL;

	// delete all sub nodes
	POSITION pos = m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			delete pNode;
			pNode = NULL;
		}
	}	
}



void CCondition::changeConditionName( CString old, const CString &newName) const
{
	CConditionNode *pNode = NULL;
	
	POSITION pos = m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			pNode->changeConditionName( old, newName);
		}
	}	
}


void CCondition::conditionDeleted( CString name)
{
	CConditionNode *pNode = NULL;

	POSITION oldpos;
	POSITION pos = m_ctConditionTree.GetHeadPosition();
	while (pos != NULL)
	{
		oldpos = pos;
		pNode = m_ctConditionTree.GetNext( pos );

		if (pNode != NULL)
		{
			if ( pNode->m_sConditionName != name)
				pNode->conditionDeleted(name);
			else
			{
				this->m_ctConditionTree.RemoveAt( oldpos );
				delete pNode;
				pNode = NULL;
			}
		}
	}
}