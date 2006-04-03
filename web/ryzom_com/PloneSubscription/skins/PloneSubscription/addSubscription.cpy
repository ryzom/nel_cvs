## Script (Python) "addSubscription"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=ptype='', id='', subscriber_uid=''
##title=Add item to subscriptions
##
# -*- coding: utf-8 -*-
if context.isPrincipiaFolderish:
    pass
else:
    if ptype=="FolderSubscription":
        context=container

from Products.CMFCore.utils import getToolByName
RESPONSE = context.REQUEST.RESPONSE
stool = getToolByName(context, 'portal_subscription')
provider = stool.getProvider()
url = '%s/%s' % (
    context.absolute_url(),
    context.getTypeInfo().getActionById('view')
    )

if provider is None:
    psm = 'psm_subscriptions_provider_cannot_be_accessed'
    return RESPONSE.redirect('%s?portal_status_message=%s' % (url, psm))

ptype = context.REQUEST.form.get('ptype','FolderSubscription')

keywords={}
if ptype == 'KeywordsSubscription':
    keywords['rpath']=context.REQUEST.form.get('rpath')
    keywords['keywords']=context.REQUEST.form.get('keywords')
    keywords['title']=context.REQUEST.form.get('keywords') # Take keywords as title
elif ptype == 'ExactSearchSubscription':
    keywords['rpath']=context.REQUEST.form.get('rpath')
    keywords['indices']=context.REQUEST.form.get('indices')
    keywords['values']=context.REQUEST.form.get('values')
    keywords['title']=context.REQUEST.form.get('keywords') # Take keywords as title
elif ptype == 'FolderSubscription':
    keywords['rpath']=context.absolute_url(relative=1)
    if context.meta_type=="Plone Site":
        # In case we are on the root folder, we have to take subscriptions from
        # the portal_subscription tool. Let's hope the Portal site will be
        # an Archetype soon
        keywords['folder']=stool.UID()
    else:
        keywords['folder']=context.UID()
    keywords['title']=context.TitleOrId()
    if context.REQUEST.form.has_key('transitions'):
        keywords['transitions']=context.REQUEST.form.get('transitions')
    if context.REQUEST.form.has_key('recursive'):
        keywords['recursive']=context.REQUEST.form.get('recursive')
    if context.REQUEST.form.has_key('workflow'):
        keywords['workflow']=context.REQUEST.form.get('workflow')
    keywords['transition']="publish"
    if context.REQUEST.form.has_key('title'):
        keywords['fullname']=context.REQUEST.form.get('title')
    if context.REQUEST.form.has_key('email'):
        keywords['email']=context.REQUEST.form.get('email')
elif ptype == 'ContentSubscription':
    keywords['rpath']=context.absolute_url(relative=1)
    keywords['content']=context.UID()
    keywords['title']=context.TitleOrId()
    if context.REQUEST.form.has_key('title'):
        keywords['fullname']=context.REQUEST.form.get('title')
    if context.REQUEST.form.has_key('email'):
        keywords['email']=context.REQUEST.form.get('email')
else:
    raise "Unknown Subscription"

psm = stool.setSubscription(id=id,
                            ptype=ptype,
                            subscriber_uid=subscriber_uid,
                            **keywords)

return RESPONSE.redirect('%s?portal_status_message=%s' % (url, psm))
