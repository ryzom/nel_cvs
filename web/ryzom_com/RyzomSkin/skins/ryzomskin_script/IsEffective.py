## Script (Python) "IsEffective"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=date
##title=
##
pastEffective = ( context.effective_date <= date or context.effective_date is None)
beforeExpiration = ( context.expiration_date >= date or context.expiration_date is None)
return pastEffective and beforeExpiration
