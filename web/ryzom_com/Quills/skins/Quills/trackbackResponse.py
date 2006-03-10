## Script (Python) "trackbackResponse"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=code, msg
##title=Generate a trackback response
##

context.REQUEST.RESPONSE.setHeader('Content-Type','text/xml')

resp = '<?xml version="1.0" encoding="iso-8859-1"?>\n'
resp += '<response>\n'
resp += '<error>' + str(code) + '</error>\n'

if msg != '':
    resp += '<message>' + msg + '</message>\n'

resp += '</response>\n'

return resp
