## Script (Python) "rendezView_editParticipant"
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
pseudo = context.REQUEST.get('pseudo')

context.setPseudo(pseudo)
context.setSeat(nbPlaces)
context.setComment(commentaire)

RESPONSE.redirect(context.absolute_url())
