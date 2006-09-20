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

path = '/'.join(context.getPhysicalPath())

###create canonical object in english
patch = context.getReleaseNote('en', path)
if patch == -1:
   return "already exist"

context.invokeFactory(id=patch[0], type_name='UrlPatchNote', title=patch[1], description='', text=patch[2])
new_obj = getattr(context, patch[0])
new_obj.setTitle(patch[1])
new_obj.setText(patch[2],mimetype='text/html')


###french translation
patch = context.getReleaseNote('fr', path)
new_obj.createTranslation(set_language="fr", current="en")
new_obj_fr = new_obj.getTranslation("fr")
new_obj_fr.setTitle(patch[1])
new_obj_fr.setText(patch[2],mimetype='text/html')


###deutch translation
patch = context.getReleaseNote('de', path)
new_obj.createTranslation(set_language="de", current="en")
new_obj_de = new_obj.getTranslation("de")
new_obj_de.setTitle(patch[1])
new_obj_de.setText(patch[2],mimetype='text/html')

#go to the patchnote view
container.REQUEST.RESPONSE.redirect(context.absolute_url())
