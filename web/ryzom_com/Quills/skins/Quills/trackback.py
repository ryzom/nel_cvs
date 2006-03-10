## Script (Python) "trackback"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=Provide trackback functionality
##

request = context.REQUEST

if not hasattr(request, 'url'):
    print context.trackbackResponse(code=1,msg="URL is missing")
    return printed

url = request.get('url')
title = request.get('title', '')
excerpt = request.get('excerpt', '')
blog_name = request.get('blog_name', '')

if not context.addTrackback(url, title, excerpt, blog_name):
   print context.trackbackResponse(code=0, msg="") 
   return printed

return True
