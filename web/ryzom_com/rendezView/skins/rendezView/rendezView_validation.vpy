## Controller Validator "rendezView_validation"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind state=state
##bind subpath=traverse_subpath
##parameters=
##title=
##
nbPlaces= context.REQUEST.get('nbPlaces')
commentaire = context.REQUEST.get('commentaire')
pseudo = context.REQUEST.get('pseudo')

if int(nbPlaces)>context.nbSeatsRestant():
   state.setError('nbPlaces', 'error')

if state.getErrors():
   state.setStatus('failure')
   return state.set(portal_status_message='Please correct the errors shown')

return state
