## Script (Python) "qna_edit_choice"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=
##title=
##
from Products.PythonScripts.standard import html_quote

form = context.REQUEST.form
keys = form.keys()
PostList = context.getPostList()
text=''
for key in keys:
   if not 'form_submit' in key:
      text += PostList[int(key)][2]

context.setText(text)

context.REQUEST.RESPONSE.redirect(context.absolute_url())
