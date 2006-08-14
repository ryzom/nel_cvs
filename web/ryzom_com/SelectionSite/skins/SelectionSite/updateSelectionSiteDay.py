## Script (Python) "updateSelectionSiteDay"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=accesslog
##title=
##
siteList = []
result = context.portal_catalog.searchResults(portal_type = 'SelectionSite')
for brains in result:
	site = brains.getObject()
	siteList.append(site.Title())
print siteList
dico = context.FanVisit(accesslog,siteList)
print dico

for brains in result:
	site = brains.getObject()
	site.addVisit(dico[site.Title()])
	site.VisitThisMonth()
	print site.Title()+'\n - Visite : '+str(site.getVisit())+'\n - Visite ce moi-ci'+str(site.getMonth())
return printed
