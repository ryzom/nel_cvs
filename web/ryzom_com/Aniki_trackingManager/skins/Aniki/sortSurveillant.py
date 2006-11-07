## Script (Python) "sortSurveillant"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=filter, capoZ
##title=
##
url = []
filter = filter.split(',')

if 'all' in filter:
	return capoZ

lang_url = {'en':'newcomer/game-download','fr':'nouveaux-venus/telecharger-le-jeu','de':'neuling-s-inn/spiel-download'}
for i in filter:
	if i in lang_url.keys():
		url.append(lang_url[i])
newcapoZ = []
for brain in capoZ:
	obj = brain.getObject()
	for u in url:
		if u in str(obj.getUrlRedirection()):
			newcapoZ.append(brain)
return newcapoZ

