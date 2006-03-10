## Script (Python) "getCategories"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=Return the proper set of Weblog Entries based on subpath
##

traverse_subpath

if len(traverse_subpath):
    path = context.absolute_url().replace(context.portal_url(),"")
    blogentries = context.portal_catalog(portal_type='WeblogEntry',
            sort_on='modified',
            sort_order='reverse',
            review_state='published',
            getEntryCategories = { 'query':traverse_subpath ,
                                   'operator':'and'},
            path = { 'query':path,
                    'level':1},
            )
    return blogentries

else:
    return []
