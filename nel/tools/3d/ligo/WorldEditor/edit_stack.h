#ifndef __EDIT_STACK_H__
#define __EDIT_STACK_H__

#include "nel/misc/types_nl.h"
#include <vector>


// This template is an edit stack class. It behaves like a stack in 3DSMax.
// You can undo/redo your class at a particular moment. You can continue
// to add elements after a undo/redo management. Take care when you reset the
// stack all elements are lost the last element stacked too. When you reset,
// you probably want to re-add the last element.

template <class CELEMENT> class CEditStack
{
	struct SStackElt
	{
		CELEMENT	Element;
		CELEMENT	*pElement;
	};

	std::vector<SStackElt>	_Stack;
	sint32					_Head, _Queue, _UndoPos;

public:

	// ---------------------------------------------------------------------------
	CEditStack ()
	{
		const int Stack_Size = 32;
		_Stack.resize (Stack_Size); // Depth of the stack
		reset ();
	}

	// ---------------------------------------------------------------------------
	CEditStack (uint32 Stack_Size)
	{
		_Stack.resize (Stack_Size);
		reset ();
	}

	// ---------------------------------------------------------------------------
	void reset ()
	{
		_Head = 0;
		_Queue = 0;
		_UndoPos = -1;
	}

	// ---------------------------------------------------------------------------
	void add (CELEMENT* pElement)
	{
		if ((sint32)((_UndoPos+1)%_Stack.size()) != _Queue)
		{
			_Queue = (_UndoPos+1)%_Stack.size();
		}

		// Stack the region
		_Stack[_Queue].Element = *pElement; // Copy the element
		_Stack[_Queue].pElement = pElement;	// Keep the memory position of the element

		_UndoPos = _Queue;
		_Queue = (_Queue+1)%_Stack.size();
		if (_Head == _Queue)
			_Head = (_Head+1)%_Stack.size();
	}

	// ---------------------------------------------------------------------------
	void undo ()
	{
		if (_UndoPos < 0)
			return;
		// Retrieve the last stacked element
		if (_UndoPos != _Head)
		{
			_UndoPos--;
			if (_UndoPos == -1)
				_UndoPos = _Stack.size()-1;
		}	
		CELEMENT *pElt = _Stack[_UndoPos].pElement;
		*pElt = _Stack[_UndoPos].Element;
	}

	// ---------------------------------------------------------------------------
	void redo ()
	{
		if (_UndoPos < 0)
			return;
		if ((sint32)((_UndoPos+1)%_Stack.size()) == _Queue)
			return;
		_UndoPos = (_UndoPos+1)%_Stack.size();
		// Retrieve the last stacked element
		CELEMENT *pElt = _Stack[_UndoPos].pElement;
		*pElt = _Stack[_UndoPos].Element;
	}

	// ---------------------------------------------------------------------------
	bool isEmpty ()
	{
		if (_Head == _Queue)
			return true;
		return false;
	}
};

#endif