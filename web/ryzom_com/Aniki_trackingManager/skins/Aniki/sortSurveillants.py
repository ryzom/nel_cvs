## Script (Python) "sortSurveillants"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=capoZ,lang=None,format=None,type=None,text=None
##title=
##

def FindTextInDesc(text,capo):
	text = text.split(';')
	result = []
	for brain in capo:
		for i in text:
			if i.upper() in str(brain.getObject().getDescription()).upper():
				result.append(brain)
	return result

def FilterByLang(lang,capo):
	lang = [lang]
	if 'all' in lang:
		return capo
	result = []
	for brain in capo
		if lang in brain.getObject().getLangs():
			result.append(brain)
	return result

def FilterByFormat(format,capo):
	format = [format]
	if 'all' in format:
		return capo
	result = []
	for brain in capo
		if format in brain.getObject().getformats():
			result.append(brain)
	return result

def FilterByType(type,capo):
	type = [type]
	if 'all' in type:
		return capo	
	result = []
	for brain in capo
		if type in brain.getObject().gettypes():
			result.append(brain)
	return result


if text:
	capoZ = FindTextInDesc(text,capoZ)
if lang:
	capoZ = FilterByLang(lang,capoZ)
if format:
	capoZ = FilterByFormat(format,capoZ)
if type:
	capoZ = FilterByType(type,capoZ)
return capoZ
