# Microsoft Developer Studio Project File - Name="ai" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ai - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ai.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ai.mak" CFG="ai - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ai - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ai - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ai - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ai - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ai - Win32 Release"
# Name "ai - Win32 Debug"
# Begin Group "agent"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\agent\actor.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\actor_script.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\agent_mailer.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\agent_object.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\agent_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\agent_script.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\agents.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\baseia.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\basic_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\digital.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\group_type.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\identifiant.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\init_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\mailbox.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\main_agent_script.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\message.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\message_script.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\msg_container.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\msg_group.cpp
# End Source File
# Begin Source File

SOURCE=.\agent\paire_type.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\actor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\actor_script.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_digital.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_mailer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_manager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_method_def.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_nombre.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_operator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_script.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agent_string.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agentexport.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\agentpack.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\baseia.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\identifiant.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\mailbox.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\main_agent_script.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\message_script.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\messagerie.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\msg_container.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\msg_group.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\agent\object_type.h
# End Source File
# End Group
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\c\ident_type.cpp
# End Source File
# Begin Source File

SOURCE=.\c\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\c\registry_class.cpp
# End Source File
# Begin Source File

SOURCE=.\c\registry_type.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\c\abstract_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\c\python_export.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\c\registry.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\c\registry_class.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\c\registry_type.h
# End Source File
# End Group
# End Group
# Begin Group "exception"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\e\ia_exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\e\nel_exception_def.h
# End Source File
# End Group
# End Group
# Begin Group "fuzzy"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fuzzy\fuzzy_script.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzycond.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzyfact.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzyrule.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzyruleset.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzyset.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzytype.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\fuzzyvar.cpp
# End Source File
# Begin Source File

SOURCE=.\fuzzy\interpret_fuzzy_controler.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzy_script.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzycond.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzyfact.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzyrule.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzyruleset.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzyset.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzytype.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\fuzzyvar.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\fuzzy\interpret_fuzzy_controler.h
# End Source File
# End Group
# End Group
# Begin Group "link"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\link\ia.cpp
# End Source File
# Begin Source File

SOURCE=.\link\static_def_init.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\link\nl_ia.h
# End Source File
# End Group
# End Group
# Begin Group "logic"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\logic\bool_assert.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\bool_cond.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\bool_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\boolval.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\clause.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\fact.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\factbase.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\fo_assert.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\fo_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\ia_assert.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\interpret_object_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\operator.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\operator_script.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\rule.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\valueset.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\var.cpp
# End Source File
# Begin Source File

SOURCE=.\logic\varset.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\bool_assert.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\bool_cond.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\bool_operator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\boolval.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\clause.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\fact.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\factbase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\fo_assert.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\fo_operator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\ia_assert.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\interpret_object_operator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\logic.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\operator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\operator_script.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\rule.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\valueset.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\var.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\logic\varset.h
# End Source File
# End Group
# End Group
# Begin Group "nimat"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\nimat\black_box.h
# End Source File
# End Group
# End Group
# Begin Group "pyserver"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pyserver\pylib.cpp
# End Source File
# Begin Source File

SOURCE=.\pyserver\pythonmethode.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\pyserver\pylib.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\pyserver\pythonsupport.h
# End Source File
# End Group
# End Group
# Begin Group "script"

# PROP Default_Filter ""
# Begin Group "cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\script\codage.cpp
# End Source File
# Begin Source File

SOURCE=.\script\code_branche_run_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\script\compilateur.cpp
# End Source File
# Begin Source File

SOURCE=.\script\constraint.cpp
# End Source File
# Begin Source File

SOURCE=.\script\constraint_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\script\constraint_find_run.cpp
# End Source File
# Begin Source File

SOURCE=.\script\constraint_stack_component.cpp
# End Source File
# Begin Source File

SOURCE=.\script\constraint_type.cpp
# End Source File
# Begin Source File

SOURCE=.\script\context_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\script\factor_type.cpp
# End Source File
# Begin Source File

SOURCE=.\script\hierarchy.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_actor.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_methodes.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_object.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_object_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_object_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_object_message.cpp
# End Source File
# Begin Source File

SOURCE=.\script\interpret_object_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\script\lexlang.cpp
# End Source File
# Begin Source File

SOURCE=.\script\lexsupport.cpp
# End Source File
# Begin Source File

SOURCE=.\script\libcode.cpp
# End Source File
# Begin Source File

SOURCE=.\script\module.cpp
# End Source File
# Begin Source File

SOURCE=.\script\objects_load.cpp
# End Source File
# Begin Source File

SOURCE=.\script\opcode.cpp
# End Source File
# Begin Source File

SOURCE=.\script\opcode_call_lib_method.cpp
# End Source File
# Begin Source File

SOURCE=.\script\opcode_call_method.cpp
# End Source File
# Begin Source File

SOURCE=.\script\opcode_ldb.cpp
# End Source File
# Begin Source File

SOURCE=.\script\opcode_lpt.cpp
# End Source File
# Begin Source File

SOURCE=.\script\opcode_register.cpp
# End Source File
# Begin Source File

SOURCE=.\script\type_def.cpp
# End Source File
# Begin Source File

SOURCE=.\script\var_control.cpp
# End Source File
# Begin Source File

SOURCE=.\script\yacc.cpp
# End Source File
# Begin Source File

SOURCE=.\script\ytable.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\nel\ai\script\codage.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\code_branche_run_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\compilateur.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\constraint.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\constraint_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\constraint_find_run.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\constraint_stack_component.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\constraint_type.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\context_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_actor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_methodes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_object_agent.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_object_list.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_object_manager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\interpret_object_message.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\lex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\lexsupport.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\libcode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\module.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\object_load.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\object_unknown.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\opcode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\opcode_call_lib_method.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\opcode_call_method.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\opcode_ldb.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\opcode_lpt.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\type_def.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\unistd.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\varstack.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\virtual_op_code.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nel\ai\script\ytable.h
# End Source File
# End Group
# Begin Group "lex&yacc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\script\lex_yacc\grammar.yacc
# End Source File
# Begin Source File

SOURCE=.\script\lex_yacc\lexicon.lex
# End Source File
# End Group
# End Group
# End Target
# End Project
