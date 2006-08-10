## Script (Python) "rendezView_addParticipant"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##
request = container.REQUEST
RESPONSE =  request.RESPONSE

nbPlaces= context.REQUEST.get('nbPlaces')
commentaires = context.REQUEST.get('commentaires')+'tttt'

#context.addParticipant(int(nbPlaces),commentaires)
print commentaires
return printed
