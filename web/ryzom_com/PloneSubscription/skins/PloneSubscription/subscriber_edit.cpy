## Controller Python Script "subscriber_edit"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind state=state
##bind subpath=traverse_subpath
##parameters=
##title=
##
# -*- coding: utf-8 -*-

from Products.CMFCore.utils import getToolByName
obj = context.aq_explicit
request = context.REQUEST
items = request.get('items', ())
state.setNextAction("redirect_to:python:request.environ.get('HTTP_REFERER')+'/subscriber_view'")

if request.has_key('form.button.Remove'):
    # Remove items
    remove_ids = []

    for item in items:
        if item.has_key('delete') and item['delete']:
            remove_ids.append(item['id'])

    obj.manage_delObjects(ids=remove_ids)
    if len(obj.objectIds()) == 0:
        portal_url = getToolByName(context, 'portal_url')
        state.setNextAction("redirect_to:string:%s/view" % request.get('default_redirect', portal_url(relative=1)))
    return state.set(status='success', portal_status_message='psm_content_removed_from_my_subscriptions')

return state.set(status='failure')
