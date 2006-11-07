## Script (Python) "newPatchNote"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=nb, url, desc
##title=
##

#on recupere le dernier id creer
path = '/'.join(context.getPhysicalPath())
result = context.portal_catalog(
			meta_type=['Surveillant',],
			path={'query':path, 'level': 0},
			sort_on='Date',
                        sort_order='reverse',
			)
lastId = result[0].getId

#creation en boucle a partir de lastId+1 jusqu'a lastId+nb

r = range(lastId+1,nb+1)
for i in r:
	context.invokeFactory(id=i, type_name='Surveillant', title=i, description=desc, urlRedirection=url)
	new_obj = getattr(context, i)
	new_obj.setTitle(i)
	new_obj.setUrlRedirection(url)

#publication
#plustard

container.REQUEST.RESPONSE.redirect(context.absolute_url())
