# Microsoft Developer Studio Project File - Name="ia" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ia - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ai.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ai.mak" CFG="ia - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ia - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ia - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ia - Win32 Release"

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
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ia - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
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

# Name "ia - Win32 Release"
# Name "ia - Win32 Debug"
# Begin Group "agent"

# PROP Default_Filter ""
# Begin Group "ag_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\agent\actor.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\actor_script.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_local_mailer.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_nombre.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_object.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_proxy_mailer.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_script.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agent_string.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\agents.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\baseai.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\basic_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\digital.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\group_type.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\ident.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\init_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\mailbox.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\main_agent_script.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\message_script.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\msg.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\msg_container.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\msg_group.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\msg_notify.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\agent\paire_type.cpp
# End Source File
# End Group
# Begin Group "ag_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\agent\actor.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\actor_script.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_digital.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_local_mailer.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_manager.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_method_def.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_nombre.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_object.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_operator.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_proxy_mailer.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_script.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agent_string.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agentexport.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\agentpack.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\baseai.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\ident.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\mailbox.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\main_agent_script.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\message_script.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\msg.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\msg_container.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\msg_group.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\msg_notify.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\object_type.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\agent\performative.h
# End Source File
# End Group
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Group "c_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\c\ident_type.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\c\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\c\registry_class.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\c\registry_type.cpp
# End Source File
# End Group
# Begin Group "c_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\c\abstract_interface.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\c\python_export.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\c\registry.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\c\registry_class.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\c\registry_type.h
# End Source File
# End Group
# End Group
# Begin Group "fuzzy"

# PROP Default_Filter ""
# Begin Group "f_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzy_script.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzycond.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzyfact.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzyrule.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzyruleset.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzyset.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzytype.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\fuzzyvar.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\fuzzy\interpret_fuzzy_controler.cpp
# End Source File
# End Group
# Begin Group "f_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzy.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzy_script.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzycond.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzyfact.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzyrule.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzyruleset.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzyset.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzytype.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\fuzzyvar.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\fuzzy\interpret_fuzzy_controler.h
# End Source File
# End Group
# End Group
# Begin Group "link"

# PROP Default_Filter ""
# Begin Group "link_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\static_def_init.cpp
# End Source File
# End Group
# Begin Group "link_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\nl_ai.h
# End Source File
# End Group
# End Group
# Begin Group "logic"

# PROP Default_Filter ""
# Begin Group "l_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\logic\ai_assert.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\bool_assert.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\bool_cond.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\bool_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\boolval.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\clause.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\fact.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\factbase.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\fo_assert.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\fo_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\goal.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\interpret_object_operator.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\operator.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\operator_script.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\rule.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\valueset.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\var.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\logic\varset.cpp
# End Source File
# End Group
# Begin Group "l_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\logic\bool_assert.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\bool_cond.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\bool_operator.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\boolval.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\clause.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\fact.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\factbase.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\fo_assert.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\fo_operator.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\goal.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\ia_assert.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\interpret_object_operator.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\logic.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\operator.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\operator_script.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\rule.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\valueset.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\var.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\logic\varset.h
# End Source File
# End Group
# End Group
# Begin Group "pyserver"

# PROP Default_Filter ""
# Begin Group "py_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\pyserver\pylib.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\pyserver\pythonmethode.cpp
# End Source File
# End Group
# Begin Group "py_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\pyserver\pylib.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\pyserver\pythonsupport.h
# End Source File
# End Group
# End Group
# Begin Group "script"

# PROP Default_Filter ""
# Begin Group "scrp_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\script\codage.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\code_branche_run_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\compilateur.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\constraint.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\constraint_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\constraint_find_run.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\constraint_stack_component.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\constraint_type.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\context_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\factor_type.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\hierarchy.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\interpret_actor.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\interpret_methodes.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\interpret_object.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\interpret_object_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\interpret_object_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\interpret_object_message.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\lexlang.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\lexsupport.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\libcode.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\module.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\objects_load.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\opcode.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\opcode_call_lib_method.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\opcode_call_method.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\opcode_ldb.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\opcode_lpt.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\opcode_register.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\script_debug_source.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\test_method.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\type_def.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\var_control.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\yacc.cpp
# End Source File
# Begin Source File

SOURCE=.\ai\script\ytable.cpp
# End Source File
# End Group
# Begin Group "scrp_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\script\codage.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\code_branche_run_debug.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\compilateur.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\constraint.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\constraint_debug.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\constraint_find_run.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\constraint_stack_component.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\constraint_type.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\context_debug.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_actor.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_methodes.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_object.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_object_agent.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_object_list.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_object_manager.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\interpret_object_message.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\lex.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\lexsupport.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\libcode.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\module.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\object_load.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\object_unknown.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\opcode.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\opcode_call_lib_method.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\opcode_call_method.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\opcode_ldb.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\opcode_lpt.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\script_debug_source.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\test_method.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\type_def.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\unistd.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\varstack.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\virtual_op_code.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\script\ytable.h
# End Source File
# End Group
# Begin Group "lex&yacc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai\script\lex_yacc\grammar.yacc
# End Source File
# Begin Source File

SOURCE=.\ai\script\lex_yacc\lexicon.lex
# End Source File
# End Group
# End Group
# Begin Group "exception"

# PROP Default_Filter ""
# Begin Group "e_cpp"

# PROP Default_Filter ""
# End Group
# Begin Group "e_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\ai\e\ai_exception.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\ai\e\nel_exception_def.h
# End Source File
# End Group
# End Group
# End Target
# End Project
