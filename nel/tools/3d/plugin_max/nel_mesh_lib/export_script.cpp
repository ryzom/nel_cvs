/** \file export_script.cpp
 * Export script utility from 3dsmax
 *
 * $Id: export_script.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdafx.h"
#include "export_nel.h"



bool CExportNel::scriptEvaluate (Interface *ip, char *script, void *out, TNelScriptValueType type)
{
	BOOL result=TRUE;
	init_thread_locals();
	four_typed_value_locals(Parser* parser,Value* code,Value* result,StringStream* source);
	
	vl.parser = new Parser ();
	vl.source = new StringStream (script);

	vl.source->log_to(NULL);
	save_current_frames();
	try
	{
		vl.source->flush_whitespace();
		vl.code = vl.parser->compile_all(vl.source);
		vl.result = vl.code->eval();
		
		vl.source->flush_whitespace();
		vl.source->close();

		// No prb ?
		if (vl.result!=FALSE)
		{
			switch (type)
			{
			case scriptFloat:
				*(float*)out=vl.result->to_float();
				break;
			}
		}
	}
	catch (...)
	{
		restore_current_frames();
		result=FALSE;
		vl.source->close();
	}
	pop_value_locals();
	return (result!=FALSE);
}
