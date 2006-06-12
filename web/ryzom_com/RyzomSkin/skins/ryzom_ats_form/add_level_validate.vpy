# Example validator code

# validate your variables here

levelM = context.REQUEST.get('levelM')
levelR = context.REQUEST.get('levelR')
levelO = context.REQUEST.get('levelO')
levelD = context.REQUEST.get('levelD')
levelC = context.REQUEST.get('levelC')
levelH = context.REQUEST.get('levelH')
motivation = context.REQUEST.get('motivation')




levelM = context.REQUEST.get('levelM', None)
if not levelM:
   state.setError('levelM', 'Please enter a value for Melee fight')
else:
   try:
       levelM = int(levelM)
   except (ValueError, TypeError):
       state.setError('levelM', 'Please enter an integer')



levelR = context.REQUEST.get('levelR', None)
if not levelR:
   state.setError('levelR', 'Please enter a value for Range fight')
else:
   try:
       levelR = int(levelR)
   except (ValueError, TypeError):
       state.setError('levelR', 'Please enter an integer')







levelD = context.REQUEST.get('levelD', None)
if not levelD:
   state.setError('levelD', 'Please enter a value for Defensive Magic')
else:
   try:
       levelD = int(levelD)
   except (ValueError, TypeError):
       state.setError('levelD', 'Please enter an integer')





levelC = context.REQUEST.get('levelC', None)
if not levelC:
   state.setError('levelC', 'Please enter a value for Crafter')
else:
   try:
       levelC = int(levelC)
   except (ValueError, TypeError):
       state.setError('levelC', 'Please enter an integer')



levelH = context.REQUEST.get('levelH', None)
if not levelH:
   state.setError('levelH', 'Please enter a value for Harvester')
else:
   try:
       levelH = int(levelH)
   except (ValueError, TypeError):
       state.setError('levelH', 'Please enter an integer')
       
       




if state.getErrors():
   # An error has occurred.  Set the status to 'failure' 
   # (the default initial status is 'success')
   state.setStatus('failure')
   # Return the state and pass the variable portal_status_message in the REQUEST
   return state.set(portal_status_message='Please correct the errors shown')

# No errors.  Always make sure to return the ControllerState object
return state


