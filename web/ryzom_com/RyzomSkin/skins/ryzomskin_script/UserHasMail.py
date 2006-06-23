## Script (Python) "UserHasMail"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=author
##title=
##
from Products.CMFCore.utils import getToolByName

mail = ''

#on recupere l'outils
mtool = getToolByName(context, 'portal_membership')

#on recupere l'auteur
member = mtool.getMemberById(author)

if member:
   mail = member.getProperty('email')
return test(mail != '',1,0)
