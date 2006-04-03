## Controller Python Script "my_subscriptions_edit"
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
""" Remove subscriptions from various subscriber """

from Products.CMFCore.utils import getToolByName
request = context.REQUEST
items = request.get('items', ())
state.setNextAction("redirect_to:python:request.environ.get('HTTP_REFERER')+'/my_subscriptions'")

tool_id = request.get('tool_id', '')
if tool_id == 'portal_subscription':
    portal_status_message = 'psm_content_removed_from_my_subscriptions'
else:
    return state.set(status='failure')

if request.has_key('form.button.Remove'):
    # Remove items
    remove_ids = {}

    for item in items:
        subscriber = item['parent']
        if item.has_key('delete') and item['delete']:
            if remove_ids.has_key(subscriber):
                remove_ids[subscriber].append(item['id'])
            else:
                remove_ids[subscriber] = [item['id']]

    uid_catalog = getToolByName(context, 'uid_catalog')
    for (subscriber_uid, ids) in remove_ids.items():
        subscriber = uid_catalog(UID=subscriber_uid)[0].getObject()
        subscriber.manage_delObjects(ids=ids)
    return state.set(status='success', portal_status_message=portal_status_message)

return state.set(status='failure')
