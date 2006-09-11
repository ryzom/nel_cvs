## Script (Python) "newPatchNote"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##
#on recupere les donnees de la base

lang = "en"
path = '/'.join(context.getPhysicalPath())
patch_en = context.patchNoteQuery(lang='en')

#detect of existing last  patch-note
if context.portal_catalog(id='patch-'+patch_en[0][7], meta_type='UrlPatchNote', path={'query':path, 'level': 0}):
   return "already exist"

#detect of serverside patch
serverside=''
if 'server' in patch_en[0][9]:
	serverside='b'

patchId = 'patch-'+patch_en[0][7]+serverside
patchTitle = 'patch '+patch_en[0][7]+serverside
patchText = patch_en[0][4]

context.invokeFactory(id=patchId, type_name='UrlPatchNote', title=patchTitle, description='', text=patchText)
new_obj = getattr(context, patchId)
new_obj.setTitle(patchTitle)
new_obj.setText(patchText,mimetype='text/html')

#french translation
patch_fr = context.patchNoteQuery(lang='fr')

patchId = 'patch-'+patch_fr[0][7]+serverside
patchTitle = 'patch '+patch_fr[0][7]+serverside
patchText = patch_fr[0][4]

new_obj.createTranslation(set_language="fr", current="en")
new_obj_fr = new_obj.getTranslation("fr")
new_obj_fr.setTitle(patchTitle)
new_obj_fr.setText(patchText,mimetype='text/html')

#deutch translation
patch_de = context.patchNoteQuery(lang='de')

patchId = 'patch-'+patch_de[0][7]+serverside
patchTitle = 'patch '+patch_de[0][7]+serverside
patchText = patch_de[0][4]

new_obj.createTranslation(set_language="de", current="en")
new_obj_de = new_obj.getTranslation("de")
new_obj_fr.setTitle(patchTitle)
new_obj_fr.setText(patchText,mimetype='text/html')

request = container.REQUEST
RESPONSE =  request.RESPONSE
RESPONSE.redirect(context.absolute_url())
