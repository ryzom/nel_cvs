
Dependencies
============

Required Products
------------------

* Plone 2.0.5
* Archetypes 1.3.3
* ATContentTypes 0.2.0
* ATReferenceBrowserWidget 1.0.1
* ExtendedPathIndex 2.1
* DataGridField 0.2 (1.0 seems to work but not really tested)
* PloneMaintenance 1.1
  - Make sure to install PloneMaintenance before PloneSubscription
  - Really recommended, but requirement is not enforced
  - Can be useful to send out scheduled notifications automatically

Installation
============

Installing ATContentTypes with Plone 2.0.5
------------------------------------------

* Unpack it into your Zope Products Folder
* Restart
* Transform CMF content with  switchCMF2ATCT (Set ATCT as default content types)

Installing PloneMaintenance
---------------------------

* Unpack it into your Zope Products Folder
* Restart Zope
* Use portal_quickinstaller to install the PloneMaintenance in ZMI (or use plone_setup in pmi)
* Setup a scheduler interface as said in PloneMaintenance/README.txt

Installing PloneSubscription
----------------------------

* Unpack it into your Zope Products Folder
* Restart Zope
* Use portal_quickinstaller to install the PloneSubscription in ZMI (or use plone_setup in pmi)


Overview
========

  PloneSubscription: A Plone tool for managing subscriptions

  The tool allows members and administrators to manage user and group subscription
  for content as well as for keyword searches. Content subscription can be done
  recursively and with the possibility to filter for documents that were recently
  subject to a workflow transition.

  There is a special tool for managing mail templates for subscription mails
  sent by PloneSubscription. The template system is fairly simple for the moment,
  but we are looking for improvement proposals to make it more powerful.

  Further PloneSubscription information can be found at http://ingeniweb.sourceforge.net/Products/PloneSubscription

Usage
=====

  Plone members will find a new icon document action (on the right side under
  the title), which allows them to quickly subscribe to content. Subscription
  is done with a single click, and doesn't disturb the member any further.

  Users can check their subscriptions by clicking on "my subscriptions" link,
  available in the user action box. The link to "my subscriptions" is only
  shown when a user is already subscribed to something.

  Another way of subscribing to content is after doing a search in Plone. In
  a special portlet, the member can choose to subscribe to the search he just
  made. This feature is only available for a simple Plone search, and will
  ignore any advanced search options that may have been chosen. 

  Subscriptions can also be managed on a per group basis, but as this is an
  advanced feature it is only available for members who explicitly get the
  "PlacelessSubscription: Manage Content" permission. 

  With this permission, an administrative user can add group subscriptions
  in just three clicks. First click on the "grouped subscription" tab on the
  current content, select the groups to subscribe, and click on save, and you
  are done.

  The permission also allows access to the PloneSubscription tool in the
  Plone Control Panel.

  Some noteworthy options of the PloneSubscription tool are:

    The 'show actions' option, which adds extra tabs on folderish objects
    for quickly adding subsciption and mail template providers locally.
    This is mostly interesting while doing through the web development.

    The 'periodicity' defines after which period of time the subscriptions are sent.
    You can send hourly, daily, weekly and yearly subscriptions.

    The 'Base url' field is used to replace a url like 'http://127.0.0.1:8080/plone' by
    'http://mydomain.com/' in the link provided in the sent mails.

  Mail sent is managed with templates stored in a specific repository. It's easy 
  to add new variables to the template namespace. The next release of PloneSubscription
  will focus on a refactoring of these templates so that they are more subscription
  centric instead of type centric.

Advanced Usage

  Your users should not need access the subscription objects themselves, so 
  the "PlacelessSubscription: Manage Content" permission is not given to Members
  any more.
  
  The best practice is to make sure the subscriptions are already added the
  right way by passing in the correct parameters as it is done in the portlet:
  
    '/addSubscription?ptype='+ptype + '&recursive=1&workflow=1&transitions:list=publish
    
  In this case you need to set the ptype to the subscription type you want and
  specify the parameters in advance.
  
  

  
  


