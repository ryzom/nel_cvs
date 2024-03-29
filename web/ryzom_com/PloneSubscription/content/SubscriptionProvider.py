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
$Id: SubscriptionProvider.py,v 1.2 2006/04/19 14:36:55 bernard Exp $
"""

__author__  = ''
__docformat__ = 'restructuredtext'


# Python imports
from AccessControl import ClassSecurityInfo

# Zope imports
from Acquisition import aq_base

# CMF imports
from Products.CMFCore  import CMFCorePermissions
from Products.CMFCore.utils import getToolByName

# Archetypes imports
from Products.Archetypes.public import *

# Products imports
from Products.PloneSubscription.config import PROJECTNAME
from Products.PloneSubscription import SubscriptionPermissions
from Products.PloneSubscription.SubscriptionCatalog import SubscriptionCatalog, manage_addSubscriptionCatalog


schema = BaseBTreeFolderSchema.copy()

class SubscriptionProvider(BaseBTreeFolder):
    """Subscription Provider with BTree"""

    __implements__ = BaseBTreeFolder.__implements__

    portal_type = meta_type = 'SubscriptionProvider'
    archetype_name = 'SubscriptionProvider'
    immediate_view = 'base_view'
    default_view   = 'base_view'
    global_allow = True
    schema = schema
    allowed_content_types = ('AnonymousSubscriber','UserSubscriber', 'GroupSubscriber')
    # XXX use property with Zope 2.8 and ZODB 3.4 only
    #allowed_content_types = property(getProviderAllowedContentTypes())
    include_default_actions = 0
    security = ClassSecurityInfo()

    actions = (
        {
        'id'            : 'view',
        'name'          : 'View',
        'action'        : 'string:${object_url}/base_view',
        'permissions'   : (SubscriptionPermissions.ManageSubscriptionContent, ),
        'category'      : 'object',
        'visible'       : 1,
        },
        {
        'id'            : 'edit',
        'name'          : 'Edit',
        'action'        : 'string:${object_url}/base_edit',
        'permissions'   : (SubscriptionPermissions.ManageSubscriptionContent, ),
        'category'      : 'object',
        'visible'       : 1,
        },
    )

registerType(SubscriptionProvider, PROJECTNAME)
