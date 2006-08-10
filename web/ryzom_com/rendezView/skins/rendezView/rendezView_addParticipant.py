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
commentaire = context.REQUEST.get('commentaire')

context.addParticipant(int(nbPlaces),commentaire)

RESPONSE.redirect(context.absolute_url())
