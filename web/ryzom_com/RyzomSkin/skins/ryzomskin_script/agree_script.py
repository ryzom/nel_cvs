## Script (Python) "agree_script"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=lang, type
##title=
##
FDLen="By clicking the \"OK\" button, you hereby agree to grant permission to copy, distribute and/or modify this document under the terms of the GNU Free Documentation License, Version 1.2 or any later version published by the Free Software Foundation; with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.  A copy of the license can be found <a href=\"/support/policies/gnu-free-documentation-license\">here</a> and the original version of the license can be found <a href=\"http://www.gnu.org/copyleft/fdl.html\">here</a>.<br />"
FDLfr=FDLen
FDLde=FDLen

FanArten="By clicking the \"OK\" button, you hereby agree with the terms of the <a href=\"/support/policies/fan-art-submissions-license-agreement\">Fan Art Submissions License Agreement</a>. Otherwise, click \"Cancel\".<br />"
FanArtfr=FanArten
FanArtde=FanArten

if type in ['ATImage','ATFile']:
  if lang=='fr':
    text=FanArtfr
  if lang=='en':
    text=FanArten
  if lang=='de':
    text=FanArtde
else:
  if lang=='fr':
    text=FDLfr
  if lang=='en':
    text=FDL1en
  if lang=='de':
    text=FDL1de
return text
