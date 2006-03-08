## Script (Python) "isOfficial (don't use it)"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=lang, type
##title=
##

text1en="By clicking the \"OK\" button, you hereby agree to grant permission to copy, distribute and/or modify this document under the terms of the GNU Free Documentation License, Version 1.2 or any later version published by the Free Software Foundation; with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.  A copy of the license can be found <a href=\"/support/policies/gnu-free-documentation-license\">here</a> and the original version of the license can be found <a href=\"http://www.gnu.org/copyleft/fdl.html\">here</a>.<p />"
text1fr=text1en+'fr'
text1de=text1en+'de'

text2en="By clicking the \"OK\" button, you hereby agree with the terms of the <a href=\"/support/policies/fan-art-submissions-license-agreement\">Fan Art Submissions License Agreement</a>. Otherwise, click \"Cancel\".<p />"
text2fr=text2en+'fr'
text2de=text2en+'de'

if type in ['ATDocument','ATNewsItem','ATEvent','WeblogEntry']:
  if lang=='fr':
    text=text1fr
  if lang=='en':
    text=text1en
  if lang=='de':
    text=text1de
else:
  if lang=='fr':
    text=text2fr
  if lang=='en':
    text=text2en
  if lang=='de':
    text=text2de
return text
