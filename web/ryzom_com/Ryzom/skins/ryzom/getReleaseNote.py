## Script (Python) "getReleaseNote"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=lang='en', path
##title=
##
patch = context.patchNoteQuery(lang=lang)

#detect of existing last patch-note
if lang=='en' and context.portal_catalog(id='patch-'+patch[0][7], meta_type='UrlPatchNote', path={'query':path, 'level': 0}):
   return -1

#detect of serverside patch
serverside=''
if 'server' in patch[0][9]:
	serverside='b'

Id = 'patch-'+patch[0][7]+serverside
Title = 'Patch '+patch[0][7]+serverside
Text = patch[0][4]
if Text == '':
   Text = patch[0][3]

return [Id,Title,Text]
