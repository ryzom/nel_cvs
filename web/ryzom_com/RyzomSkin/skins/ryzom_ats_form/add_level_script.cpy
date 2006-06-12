## Controller Python Script "newsletters_mailing_script"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind state=state
##bind subpath=traverse_subpath
##parameters=levelM, levelR, levelO, levelD, levelC, levelH, motivation
##title=


# Get the values from the REQUEST
request = container.REQUEST
response =  request.RESPONSE

levelM = context.REQUEST.get('levelM')
levelR = context.REQUEST.get('levelR')
levelO = context.REQUEST.get('levelO')
levelD = context.REQUEST.get('levelD')
levelC = context.REQUEST.get('levelC')
levelH = context.REQUEST.get('levelH')
motivation = context.REQUEST.get('motivation')


# New tester being editted


try:
       insertResult = context.ryzom_ats_insert( mf= levelM, rf= levelR, om= levelO, dm= levelD, cr= levelC, ha= levelH, text=motivation).dictionaries()
  
except :
       print 'Error adding a new player data.'
       return printed


#
# End of processing form inputs.
#


# Show successfull message (normally a generic ZPT)

#print '<p>Edit successful</p>'
#print '<p>Click <a href="' + request.URL + '">here</a> to return</p>'
#return printed

#state.setNextAction('redirect_to:string:http://localhost/ryzom')

return state

#return state.set(status='success', portal_status_message=toolResponse)






