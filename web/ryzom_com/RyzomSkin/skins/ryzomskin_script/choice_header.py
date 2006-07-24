## Script (Python) "choice_header"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=directory
##title=
##
from random import choice

results=[]
headers=[]
#on recupere la langue de l'utilisateur
lang = context.portal_languages.getLanguageBindings()[0]                    
#on recupere l'object contenant les headers
dir=context.restrictedTraverse(directory)

#on recupere les objets du conteneur
results= dir.objectIds()

#on trie par langue
for img in results:   
   if lang in img.split('_')[2]:      
      headers.append(img)

tmp=choice(headers)
return tmp

