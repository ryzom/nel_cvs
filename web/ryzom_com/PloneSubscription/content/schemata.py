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
$Id: schemata.py,v 1.1 2006/04/03 13:44:15 bernard Exp $
"""

__author__  = ''
__docformat__ = 'restructuredtext'

# Zope imports
from DateTime import DateTime

# CMF imports
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions

# Archetypes import
from Products.Archetypes.public import *

# Products imports
from Products.PloneSubscription import SubscriptionPermissions


# Base Subscribers schema
BaseSubscriberSchema = BaseFolderSchema.copy()

# Base Subcription schema
BaseSubscriptionSchema = BaseSchema.copy() + Schema((
    StringField(
        'rpath',
        required=True,
        searchable=True,
        default='/',
        read_permission=SubscriptionPermissions.ViewSubscriptionContent,
        write_permission=SubscriptionPermissions.EditSubscriptionContent,
        widget=StringWidget(
            visible={ 'view': 'visible', 'edit': 'visible',},
            label="Relative path",
            label_msgid="label_subscription_rpath",
            description="Relative path from plone site root.",
            description_msgid="help_subscription_rpath",
            i18n_domain="plonesubscription",
            ),
        ),
    DateTimeField(
        'last_send',
        required=True,
        searchable=True,
        default=DateTime(),
        read_permission=SubscriptionPermissions.ViewSubscriptionContent,
        write_permission=SubscriptionPermissions.ManageSubscriptionContent,
        widget=CalendarWidget(
            visible={ 'view': 'visible', 'edit': 'hidden',},
            label="Last send datetime",
            label_msgid="label_subscription_last_send",
            description="Last DateTime this subscription was activated.",
            description_msgid="help_subscription_last_send",
            i18n_domain="plonesubscription",
            ),
        ),
    ),)
BaseSubscriptionSchema['id'].widget.visible = { 'view': 'visible', 'edit': 'hidden',}
BaseSubscriptionSchema['title'].widget.visible = { 'view': 'visible', 'edit': 'hidden',}
BaseSubscriptionSchema['title'].required = False
BaseSubscriptionSchema['description'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}

# Forms specific schema

FormAddGroupSubscriptionSchema = Schema((
    StringField(
        'groupSubscriber',
        required=True,
        searchable=False,
        default=[],
        multiValued=True,
        vocabulary='getGroupSubscriberVocabulary',
        enforceVocabulary=True,
        read_permission=SubscriptionPermissions.ManageSubscriptionContent,
        write_permission=SubscriptionPermissions.ManageSubscriptionContent,
        widget=MultiSelectionWidget(
            visible={ 'view': 'visible', 'edit': 'visible',},
            label="Grouped subscribers",
            label_msgid="label_grouped_subscriber",
            description="Grouped subcribers available",
            description_msgid="help_grouped_subscriber",
            i18n_domain='plonesubscription',
            ),
        ),
    ),)
