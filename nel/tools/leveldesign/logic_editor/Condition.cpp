// Condition.cpp: implementation of the CCondition class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "condition.h"
#include "logic/logic_condition.h"

#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace std;

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



//-----------------------------------------------------
//	cConditionNodeToCLogicConditionNode (Editor --> Service)
//
//-----------------------------------------------------
void cConditionNodeToCLogicConditionNode(CConditionNode& conditionNode, CLogicConditionNode& logicConditionNode )
{
	// if this node is a terminator node
	if( conditionNode.m_type == CConditionNode::TERMINATOR )
	{
		logicConditionNode.Type = CLogicConditionNode::TERMINATOR; 
		
	}
	else
	// this node is a logic node
	{
		logicConditionNode.Type = CLogicConditionNode::LOGIC_NODE;
				
		// part 1 : a logic block(not/comparison/subcondition)
		switch( conditionNode.m_type )
		{
			case CConditionNode::NOT :
			{
				logicConditionNode.LogicBlock.Type = CLogicConditionLogicBlock::NOT;
				
			}
			break;

			case CConditionNode::COMPARISON :
			{
				logicConditionNode.LogicBlock.Type = CLogicConditionLogicBlock::COMPARISON;
				
				logicConditionNode.LogicBlock.ComparisonBlock.VariableName = string( (LPCSTR)conditionNode.m_sVariableName );
				logicConditionNode.LogicBlock.ComparisonBlock.Operator = string( (LPCSTR)conditionNode.m_sOperator );
				logicConditionNode.LogicBlock.ComparisonBlock.Comparand = (sint64)conditionNode.m_dComparand;
			}
			break;
			
			case CConditionNode::SUB_CONDITION :
			{
				logicConditionNode.LogicBlock.Type = CLogicConditionLogicBlock::SUB_CONDITION;
				
				logicConditionNode.LogicBlock.SubCondition = string( (LPCSTR)conditionNode.m_sConditionName );
			}
			break;
		}

		// part 2 : a condition sub tree
		POSITION pos;
		for( pos = conditionNode.m_ctSubTree.GetHeadPosition(); pos != NULL; )
		{
			CConditionNode * pConditionNode = conditionNode.m_ctSubTree.GetNext( pos );
			CLogicConditionNode logicConditionNodeTmp;
			cConditionNodeToCLogicConditionNode( *pConditionNode, logicConditionNodeTmp );
			logicConditionNode.addNode( logicConditionNodeTmp );
		}
	}

} // cConditionNodeToCLogicConditionNode //




//-----------------------------------------------------
//	cConditionToCLogicCondition (Editor --> Service)
//
//-----------------------------------------------------
void cConditionToCLogicCondition( CCondition& condition, CLogicCondition& logicCondition )
{
	// condition name
	logicCondition.setName( string( (LPCSTR)condition.m_sName ) );

	// nodes
	POSITION pos;
	for( pos = condition.m_ctConditionTree.GetHeadPosition(); pos != NULL; )
	{
		// get the node
		CConditionNode * pConditionNode = condition.m_ctConditionTree.GetNext( pos );
		
		// convert the node
		CLogicConditionNode logicConditionNode;
		cConditionNodeToCLogicConditionNode( *pConditionNode,logicConditionNode );

		// add the node
		logicCondition.addNode( logicConditionNode );
	}

} // cConditionToCLogicCondition //








//-----------------------------------------------
//	cLogicConditionToCCondition // TODO : use a recursive function (Service --> Editor)
//
//-----------------------------------------------
void cLogicConditionToCCondition( CLogicCondition& logicCondition, CCondition& condition )
{
	// condition name
	condition.m_sName = CString( logicCondition.getName().c_str() );

	// condition tree
	vector<CLogicConditionNode>::iterator itNode;
	for( itNode = logicCondition.Nodes.begin(); itNode != logicCondition.Nodes.end(); ++itNode )
	{
		CConditionNode * node = new CConditionNode();

		// terminator node
		if((*itNode).Type == CLogicConditionNode::TERMINATOR)
		{
			node->m_type = CConditionNode::TERMINATOR;
		}
		// logic block with condition sub tree
		else
		{
			switch( (*itNode).LogicBlock.Type )
			{
/*
				case CLogicConditionLogicBlock::NOT :
				{
					node->m_type = CConditionNode::NOT;
				};
				break;
*/

				case CLogicConditionLogicBlock::COMPARISON :
				{
					node->m_type = CConditionNode::COMPARISON;
					
					node->m_sVariableName = CString((*itNode).LogicBlock.ComparisonBlock.VariableName.c_str());
					node->m_sOperator = CString((*itNode).LogicBlock.ComparisonBlock.Operator.c_str());
					node->m_dComparand = (double)(*itNode).LogicBlock.ComparisonBlock.Comparand;

				};
				break;

/*
				case CLogicConditionLogicBlock::SUB_CONDITION :
				{
					node->m_type = CConditionNode::SUB_CONDITION;
					node->m_sConditionName = CString((*itNode).LogicBlock.SubCondition.c_str());
				};
				break;
*/

				// TEMP : other case are not managed yet, the node is set to termiator
				default :
				{
					node->m_type = CConditionNode::TERMINATOR; //TEMP
				}

			}
			
			// TODO : subtree
			//initCConditionSubtree( m_ctSubTree );
			
		}

		// add the new node 
		condition.m_ctConditionTree.AddTail( node );
	}

} // cLogicConditionToCCondition //


