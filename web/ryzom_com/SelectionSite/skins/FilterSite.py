## Script (Python) "FilterSite"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##

def getSite():



	siteList = []
	result = context.portal_catalog.searchResults(portal_type = 'SelectionSite')
	for brains in result:
      		site = brains.getObject()
      		siteList.append(site.Title())
      
	#return siteList
	dico = FanVisitg1(siteList)
	
	for brains in result:
		site = brains.getObject()
		site.updateDico(dico[site.Title])