## Script (Python) "qna_edit_request"
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

try:
	context.setId(form['title'])
except:
	pass
context.setTitle(form['title'])
context.setDescription(form['description'])
context.setDateStart(form['dateStart'])
context.setDateEnd(form['dateEnd'])
context.setFilter(form['filter'])

context.createPostList()

context.REQUEST.RESPONSE.redirect(context.absolute_url()+'/qna_edit_choice_form')
