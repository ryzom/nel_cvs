# -*- coding: utf-8 -*-
## PloneSubscription
## A Plone tool supporting different levels of subscription and notification
## Copyright (C)2006 Ingeniweb

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; see the file COPYING. If not, write to the
## Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

"""
PloneSubscription specific permissions.
"""
__version__ = "$Revision: 1.1 $"
# $Source: /mnt/x/wsl/cvsexp3/cvs/code/web/ryzom_com/PloneSubscription/SubscriptionPermissions.py,v $
# $Id: SubscriptionPermissions.py,v 1.1 2006/04/03 13:37:16 bernard Exp $
__docformat__ = 'restructuredtext'


from Products.CMFCore.CMFCorePermissions import setDefaultRoles

AddSubscriptionContent = 'PlacelessSubscription: Add Content'
ManageSubscriptionContent = 'PlacelessSubscription: Manage Content'
ViewSubscriptionContent = 'PlacelessSubscription: View Content'
EditSubscriptionContent = 'PlacelessSubscription: Edit Content'
ManageSubscriptions = "PlacelessSubscription: Manage Subscriptions"


ViewSubscriptionRole = 'SubscriptionViewer'

# Set up default roles for permissions
setDefaultRoles(AddSubscriptionContent, ('Anonymous', 'Member', 'Manager'))
setDefaultRoles(ManageSubscriptionContent, ('Manager',))
setDefaultRoles(ViewSubscriptionContent, ('Anonymous', 'Manager', 'Owner',))
setDefaultRoles(EditSubscriptionContent, ('Manager',))
setDefaultRoles(ManageSubscriptions, ('Manager',))
