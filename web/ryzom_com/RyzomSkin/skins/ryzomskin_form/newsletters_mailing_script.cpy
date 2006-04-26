## Controller Python Script "newsletters_mailing_script"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind state=state
##bind subpath=traverse_subpath
##parameters=
##title=
##
# Get the values from the REQUEST
email = context.REQUEST.get('email')
daily = context.REQUEST.get('daily')
monthly = context.REQUEST.get('monthly')

#Get the language
lang = context.portal_languages.getLanguageBindings()[0]

#Build the email
urldaily="news-"+lang+"-subscribe@newsletter.nevrax.com"
urlmonthly="newsletter-"+lang+"-subscribe@newsletter.nevrax.com"
body = 'Subscribe'
mSubject = 'Subscribe'

try:
   mailhost=context.MailHost
except:
   raise AttributeError, "Cannot find a Mail Host object"

if daily != None:
   mFrom = email
   mTo = urldaily
   mailhost.send(body, mTo, mFrom, mSubject)

if monthly != None:
   mFrom = email
   mTo = urlmonthly
   mailhost.send(body, mTo, mFrom, mSubject)

# Always make sure to return the ControllerState object
return state
