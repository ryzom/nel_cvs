## Script (Python) "testpy"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=file
##title=
##
try:
   date = str(DateTime())[:10]
   result = context.checkDate(date=date,file=file)   
   if len(result) == 0:
      context.insertDate(date=date,file=file)
   context.incremente_dl(date=date,file=file)
except:
   pass
container.REQUEST.RESPONSE.redirect(file)

