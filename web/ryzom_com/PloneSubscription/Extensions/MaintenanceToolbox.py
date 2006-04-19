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
Various function for PloneMaintenance calls
"""
__version__ = "$Revision: 1.2 $"
# $Source: /mnt/x/wsl/cvsexp3/cvs/code/web/ryzom_com/PloneSubscription/Extensions/MaintenanceToolbox.py,v $
# $Id: MaintenanceToolbox.py,v 1.2 2006/04/19 14:36:55 bernard Exp $
__docformat__ = 'restructuredtext'


from StringIO import StringIO

# We need to run this script with all permissions
from AccessControl import ClassSecurityInfo, getSecurityManager
from AccessControl.SecurityManagement import newSecurityManager
from AccessControl.User import UnrestrictedUser

from Products.CMFCore.utils import getToolByName

# ############################################################
# 1/ run mailing scheduler
# ############################################################

def runMailing(self):
    """Run mailing from subscription tool"""
    out = StringIO()
    sub_tool = getToolByName(self, 'portal_subscription')

    current_user = getSecurityManager().getUser()
    newSecurityManager(None, UnrestrictedUser('manager', '', ['Manager'], []))
    sub_tool.mailing()
    newSecurityManager(None, current_user)
    out.write("Subscription mailing have run\n\n")

    return out.getvalue()
