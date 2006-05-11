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
dir=context.restrictedTraverse(directory)
results= dir.objectIds()
tmp=choice(results)
return tmp
