## Script (Python) "first20chars"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=text
##title=
##

# Here, text is a string

text=text.replace('-',' ')
if(len(text)>25) :
   return text[:25]+'...'
return text
