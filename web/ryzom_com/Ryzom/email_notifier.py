## Script (Python) "email_notifier"
## Script (Python) "redirect_view_python"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=review_state
##title=
##

# This script has been designed to send email to cmf users  #
# with appropriate preferences. The script should be used   #
# in conjunction with the workflow tool.                    #
# parameters review_state                                   #
# http://plone.org/documentation/how-to/send-announcements  #
#############################################################

#############################################################
# Functions

# A nifty little function, which checks to see whether there 
# are any elements that match between two lists, and returns 
# the number of matches. Result: if the function returns true, 
# you've got a match
def isIn(list1, list2):
     y=0
     for x in list1:
         if x in list2:
             y += 1
     return y

#############################################################
# Set up a empty list of email addresses
# loop through the portal membership, pass memberId to check for
# Member role. If successful, check to see if the member has given
# permission to send email, and an area of business interest that 
# coincides with a content keyword. If successful, append the 
# list of email addresses and send them email

# Get the content object we're publishing
contentObject = review_state.object


# Start with an empty list, ready to be filled with the addressed
# of people we're dispatching to
mailList=[]

# Iterate through all the site's users
grp = context.portal_groups.getGroupById('email-notification')
if grp == None: return
for user in grp.getGroupMembers():
    memberId = user.id
    # Remember that a real name is not mandatory, so fall back to the username
    if user.fullname:
        memberName = user.fullname
    else:
        memberName = memberId

    # add them to the list of people we're emailing
    mailList.append(user.email)

    # check that we can send email via the Zope standard Mail Host
    try:
       mailhost=context.MailHost
    except:
       raise AttributeError, "Cannot find a Mail Host object"
    # Let's write an email:
    mMsg = "[" + str(contentObject.portal_type) + "] *" + contentObject.TitleOrId() + "*\n\n"
    mMsg += contentObject.Description() + "\n\n\n"
    mMsg += "Read more: " + contentObject.absolute_url() + "\n\n"
    mMsg += "-- \nXavier.\n"
    mTo = user.email
    mFrom = "Xavier Antoviaque <xavier@nevrax.com>"
    mSubj = "[" + str(contentObject.portal_type) + "] " + contentObject.TitleOrId()
    # and send it
    mailhost.send(mMsg, mTo, mFrom, mSubj)
    print mMsg


# The change in indentation signals the end of the loop, so we've
# now sent all the emails. Let's now send a confirmation that we've done it.
# We'll be building the email as a string again, but we have to convert our
# list data elements into a string before we can append the information
recipients = string.join(mailList, sep="\n")
mTo = "xavier@nevrax.com"
mMsg = "The following people were sent a link to\n"
mMsg += contentObject.absolute_url() + "\n\n"
mMsg += recipients + "\n\n"
mSubj = "Content announcement email confirmation"
mailhost.send(mMsg, mTo, mFrom, mSubj)
