# validate your variables here
daily = context.REQUEST.get('daily')
monthly = context.REQUEST.get('monthly')

if daily != None or monthly != None:
   pass
else:
   state.setError('daily', 'Please check daily and/or monthly-'+str(daily))
   state.setError('monthly', 'Please check daily and/or monthly-'+str(monthly))
#state.setError('email','+'+str(daily)+'-'+str(monthly))
if state.getErrors():
   # An error has occurred.  Set the status to 'failure' 
   # (the default initial status is 'success')
   state.setStatus('failure')
   # Return the state and pass the variable portal_status_message in the REQUEST
   return state.set(portal_status_message='Please correct the errors shown')

# No errors.  Always make sure to return the ControllerState object
return state
