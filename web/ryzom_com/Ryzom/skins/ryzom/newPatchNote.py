## Script (Python) "newPatchNote"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##
lang = "en"
patch_en = context.patchNoteQuery(lang='en')
if context.portal_catalog(id=patch_en[0][7], meta_type='UrlPatchNote'):
   return "already exist"

context.invokeFactory(id=patch_en[0][7], type_name='UrlPatchNote', description='', text=patch_en[0][4])
new_obj = getattr(context, patch_en[0][7])
new_obj.setTitle(patch_en[0][7])
new_obj.setText(patch_en[0][4],mimetype='text/html')

patch_fr = context.patchNoteQuery(lang='fr')
new_obj.createTranslation(set_language="fr", current="en")
new_obj_fr = new_obj.getTranslation("fr")
new_obj_fr.setTitle(patch_fr[0][7])
new_obj_fr.setText(patch_fr[0][4],mimetype='text/html')

patch_de = context.patchNoteQuery(lang='de')
new_obj.createTranslation(set_language="de", current="en")
new_obj_de = new_obj.getTranslation("de")
new_obj_de.setTitle(patch_de[0][7])
new_obj_de.setText(patch_de[0][4],mimetype='text/html')
