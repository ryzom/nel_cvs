/** \file lex.h
 * 
 *
 * $Id: lex.h,v 1.4 2001/01/17 16:53:18 chafik Exp $
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


#ifndef NL_LEX_H
#define NL_LEX_H

#include <strstream>

struct yy_buffer_state;

typedef int yy_state_type;
#define YY_SKIP_YYWRAP

namespace NLAISCRIPT
{
	
	
	class FlexLexer {
	public:
		virtual ~FlexLexer()	{ }

		const char* YYText()	{ return yytext; }
		int YYLeng()		{ return yyleng; }

		virtual void
			yy_switch_to_buffer( struct yy_buffer_state* new_buffer ) = 0;
		virtual struct yy_buffer_state*
			yy_create_buffer( istream* s, int size ) = 0;
		virtual void yy_delete_buffer( struct yy_buffer_state* b ) = 0;
		virtual void yyrestart( istream* s ) = 0;

		virtual int yylex() = 0;

		// Call yylex with new input/output sources.
		int yylex( istream* new_in, ostream* new_out = 0 )
			{
			switch_streams( new_in, new_out );
			return yylex();
			}

		// Switch to new input/output streams.  A nil stream pointer
		// indicates "keep the current one".
		virtual void switch_streams( istream* new_in = 0,
						ostream* new_out = 0 ) = 0;

		int lineno() const		{ return yylineno; }

		int debug() const		{ return yy_flex_debug; }
		void set_debug( int flag )	{ yy_flex_debug = flag; }		

	protected:
		char* yytext;
		int yyleng;
		int yylineno;		// only maintained if you use %option yylineno
		int yy_flex_debug;	// only has effect with -d or "%option debug"		
		
	};


	#define yyFlexLexerOnce

	class yyFlexLexer : public FlexLexer {
	public:
		// arg_yyin and arg_yyout default to the cin and cout, but we
		// only make that assignment when initializing in yylex().
		yyFlexLexer( istream* arg_yyin = 0, ostream* arg_yyout = 0 );

		virtual ~yyFlexLexer();

		void yy_switch_to_buffer( struct yy_buffer_state* new_buffer );
		struct yy_buffer_state* yy_create_buffer( istream* s, int size );
		void yy_delete_buffer( struct yy_buffer_state* b );
		void yyrestart( istream* s );

		virtual int yylex();
		virtual void switch_streams( istream* new_in, ostream* new_out );
		

	protected:
		virtual int LexerInput( char* buf, int max_size );
		virtual void LexerOutput( const char* buf, int size );
		virtual void LexerError( const char* msg );

		void yyunput( int c, char* buf_ptr );
		int yyinput();

		void yy_load_buffer_state();
		void yy_init_buffer( struct yy_buffer_state* b, istream* s );
		void yy_flush_buffer( struct yy_buffer_state* b );

		int yy_start_stack_ptr;
		int yy_start_stack_depth;
		int* yy_start_stack;

		void yy_push_state( int new_state );
		void yy_pop_state();
		int yy_top_state();

		yy_state_type yy_get_previous_state();
		yy_state_type yy_try_NUL_trans( yy_state_type current_state );
		int yy_get_next_buffer();

		istream* yyin;	// input source for default LexerInput
		ostream* yyout;	// output sink for default LexerOutput

		struct yy_buffer_state* yy_current_buffer;

		// yy_hold_char holds the character lost when yytext is formed.
		char yy_hold_char;

		// Number of characters read into yy_ch_buf.
		int yy_n_chars;

		// Points to current character in buffer.
		char* yy_c_buf_p;

		int yy_init;		// whether we need to initialize
		int yy_start;		// start state number

		// Flag which is used to allow yywrap()'s to do buffer switches
		// instead of setting up a fresh yyin.  A bit of a hack ...
		int yy_did_buffer_switch_on_eof;

		// The following are not always needed, but may be depending
		// on use of certain flex features (like REJECT or yymore()).

		yy_state_type yy_last_accepting_state;
		char* yy_last_accepting_cpos;

		yy_state_type* yy_state_buf;
		yy_state_type* yy_state_ptr;

		char* yy_full_match;
		int* yy_full_state;
		int yy_full_lp;

		int yy_lp;
		int yy_looking_for_trail_begin;

		int yy_more_flag;
		int yy_more_len;
		

		//this is a new define type for our operation
		char LastyyText[2][1024*4];
		double LastyyNum;		
		int yyLine,yyColone;

		virtual int	yywrap() = 0;

		/// Tell the _lastBlock about a newLine (for Debug)
		virtual void setNewLine() = 0;
	};
}
#endif
