## Script (Python) "isOfficial"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=author
##title=
##

user=context.acl_users.getUserByName(author);
return (user and ('group_Officials' in user.getGroups()))
