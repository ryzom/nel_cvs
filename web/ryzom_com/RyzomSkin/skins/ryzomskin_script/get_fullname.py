## Script (Python) "get_fullname"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=author
##title=
##
try:
   fullname = context.portal_membership.getMemberById(author).getProperty('fullname')
except:
   fullname = author
if fullname != '':
   return fullname
return author