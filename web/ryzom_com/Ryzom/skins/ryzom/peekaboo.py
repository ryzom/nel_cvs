## Script (Python) "peekaboo"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=text
##title=
##
#remove the peekaboo bug for ie5/6
text.replace('<div style="clear: both;"></div>','',1)
return text
