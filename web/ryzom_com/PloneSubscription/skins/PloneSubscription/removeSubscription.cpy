## Controller Python Script "removeSubscription"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind state=state
##bind subpath=traverse_subpath
##parameters=id=None
##title=Remove subscription
##
# -*- coding: utf-8 -*-

message = ""
from Products.CMFCore.utils import getToolByName
stool = getToolByName(context, 'portal_subscription')
mtool = getToolByName(context, 'portal_membership')
if mtool.isAnonymousUser():
    if id:
        # id is given, so this link comes from an email
        message = stool.removeSubscription(context, subscriber_id = id)
    else:
        email = context.REQUEST.get('email')
        if stool.getAnonymous_unsubscribe_by_email():
            message = stool.unsubscriptionMailing(context, email)
        else:
            message = stool.removeSubscription(context)
else:
    message = stool.removeSubscription(context)
return state.set(status='success', portal_status_message=message)

