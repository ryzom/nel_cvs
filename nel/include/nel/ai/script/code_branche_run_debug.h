/** \file code_branche_run_debug.h
 * A debug version of CCodeBrancheRun. It keep a trace between original source code and OpCode.
 *
 * $Id: code_branche_run_debug.h,v 1.3 2001/01/08 11:16:29 chafik Exp $
 */

/** Copyright, 2000 Nevrax Ltd.
 */

#ifndef RL_CODE_BRANCHE_RUN_DEBUG_H
#define RL_CODE_BRANCHE_RUN_DEBUG_H

#include "agent/agentpack.h"
#include "agent/agent_script.h"

#include "script/codage.h"

namespace NLAISCRIPT
{
//	typedef	std::list<CBagOfCode *> TListCode;
//	typedef	std::list<CBagOfCode *>::iterator tListCodeIter;

	/**
	 * A debug version of CCodeBrancheRun. It keep a trace between original source code and OpCode.
	 * \author Gabriel ROBERT
	 * \author Nevrax France
	 * \date 2000
	 */
	class CCodeBrancheRunDebug : public CCodeBrancheRun
	{
	public:
		static const NLAIC::CIdentType IdCodeBrancheRunDebug;

	private :
		int			*_LineInSourceCodeArray;
		std::string	_SourceFileName;

	 public:

		/** Constructor
		 *	param N Size of the CodeBranche
		 *	param op Default OpCode
		 */
		CCodeBrancheRunDebug(sint32 N,const IOpCode &op);		

		/** Constructor
		 *	param N Size of the CodeBranche
		 */
		CCodeBrancheRunDebug(sint32 N, const char* sourceCodeFileName);

		/** Constructor
		 *	param l CBagOfCode list
		 */
		CCodeBrancheRunDebug(const tListCode &l);

		 /// Copy constructor
		CCodeBrancheRunDebug(const CCodeBrancheRunDebug &l);

		/// Destructor
		virtual ~CCodeBrancheRunDebug();

		///	Init the CCodeBrancheRunDebug from a CBagOfCode list
		void initCode(const tListCode &l);

		///	Init the CCodeBrancheRunDebug from an other CCodeBrancheRunDebug
		void initCode(const CCodeBrancheRunDebug &l);

		/// Return a debug string
		void getDebugResult(char *str,CCodeContext &context) const;

		/**	The true run of this class
		 *	param self A reference to the IObjectIA which call this ICodeBranche
		 */
		const NLAIAGENT::IObjectIA::CProcessResult &run(NLAIAGENT::IObjectIA &self);

		const NLAIAGENT::IObjectIA::CProcessResult &run(CCodeContext &);

		/// Run the next IOpCode in the array
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		/// Store the linde in the source code associate with the OpCode
		void setLineCode(int ligne, int index);

		/// Return the name (& path) of the source code file.
		const char* getSourceFileName() const;

		/// Return the line number in source code of the actual opCode.
		uint16	getCurrentSourceCodeLineNumber() const;

		/// Print the current line in source code of the actual opCode.
		void printCurrentSourceCodeLine() const;

		/// Print lines between first and last in source code of the actual opCode.
		void printSourceCodeLine(int first, int last) const;

		/// \name IBasicInterface method.
		//@{
		const NLAIC::CIdentType& getType() const;
		const NLAIC::IBasicType* clone() const;
		const NLAIC::IBasicType *newInstance() const;
		virtual void save(NLMISC::IStream &);
		virtual void load(NLMISC::IStream &);
		//@}

	 private :
		 
		void initCBRD(); // Call by each constructor.
		void fixContextDebugMode(CCodeContext &P) const;
		/// Return True if the next opCode must be run.
		bool readCommandLine(CCodeContext &P, const char* commandLine);
		void printVariable (CCodeContext &P, const char* c) const;
		void printArgs (CCodeContext &P) const;
	};
}
#endif // RK_CODE_BRANCHE_RUN_DEBUG_H

/* End of code_branche_run_debug.h */
