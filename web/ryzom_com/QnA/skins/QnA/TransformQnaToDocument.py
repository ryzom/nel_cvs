## Script (Python) "TransformQnaToDocument"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=qna
##title=
##
from Products.CMFCore.utils import getToolByName
portal = getToolByName(context,'portal_url').getPortalObject()

#on recupere l'objet qna a convertir dans le context (le meme conteneur) ou on le recherche dans le catalog.
try:
   object = context.restrictedTraverse(qna)
except:
   result = context.portal_catalog(id=qna)
   object = result[0].getObject()


#on recupere les valeurs utiles du qna
if object:
   id = object.getId()
   title = object.Title()
   description = object.getDescription()
   text = object.getText()

   #on recuepre l'objet conteneur du qna a convertir
   path = '/'.join(object.getPhysicalPath()).split('/'+qna)[0]
   target = portal.restrictedTraverse(path)

   #on fabrique une nouvelle id (faire autre chose)
   newid = id + '_Document'

   #appel du constructeur et on remplis les diffÃƒÂ©rents champs
   target.invokeFactory(id=newid, type_name='Document', title=title, description=description,text=text)
   new_obj = getattr(target, newid)
   new_obj.setDescription(description)
   new_obj.setText(text,mimetype='text/html')

   request = container.REQUEST
   RESPONSE =  request.RESPONSE
   RESPONSE.redirect(new_obj.absolute_url())

else:
   print "objet "+qna+" don't exist"
   return printed
