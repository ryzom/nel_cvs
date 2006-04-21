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
$Id: Install.py,v 1.3 2006/04/21 15:02:10 bernard Exp $
"""
__author__  = ''
__docformat__ = 'restructuredtext'

# Python imports
from StringIO import StringIO

# Zope imports
from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod


# CMF imports
from Products.CMFCore.utils import getToolByName
from Products.CMFCore.CMFCorePermissions import ManagePortal, View

# Archetypes
from Products.Archetypes.Extensions.utils import installTypes
from Products.Archetypes.public import listTypes

# Product import
from Products.PloneSubscription.SubscriptionTool import SubscriptionTool
from Products.PloneSubscription.SubscriptionPermissions import \
     ViewSubscriptionRole, ViewSubscriptionContent, ManageSubscriptionContent
from Products.PloneSubscription.config import PROJECTNAME, GLOBALS, \
    subscription_prefs_configlet, subscription_task_id, subscription_script_id, \
    zope_DEPS, plone_DEPS
from Products.PloneSubscription.workflows.subscription_content_workflow import create_subscription_content_workflow

from Products.PloneSubscription.installers.utils import InstallationRunner, InstallationContext
from Products.PloneSubscription.installers.RoleInstaller import RoleInstaller
from Products.PloneSubscription.installers.ActionsInstaller import ActionsInstaller
from Products.PloneSubscription.installers.StandardPropertiesInstaller import StandardPropertiesInstaller
from Products.PloneSubscription.installers.ToolInstaller import ToolInstaller
from Products.PloneSubscription.installers.SkinLayersInstaller import SkinLayersInstaller
from Products.PloneSubscription.installers.ConfigletsInstaller import ConfigletsInstaller
from Products.PloneSubscription.installers.PortletInstaller import PortletInstaller
from Products.PloneSubscription.installers.WorkflowInstaller import WorkflowInstaller
from Products.PloneSubscription.installers.CatalogInstaller import CatalogInstaller

from Products.PloneSubscription.SubscriptionCatalog import SubscriptionCatalog
from Products.PloneSubscription.SubscriptionCatalog import manage_addSubscriptionCatalog

def getRunners():

    installers = []

    # grouped subscription needs a role
    ri = RoleInstaller(ViewSubscriptionRole, model='Anonymous',
                       allowed=(ViewSubscriptionContent, View))
    installers.append(ri)

    # PloneSubscription actions - portal_actions
    pf_actions = (
        {
            'id': 'my_subscriptions',
            'name': 'action_my_subscriptions',
            'action':'string:${portal_url}/my_subscriptions',
            'condition': 'python:member and portal.portal_subscription.hasSubscription()',
            'permission': View,
            'category':'user',
            'visible': 1
            },
        {
            'id': 'addSubscription',
            'name': 'action_add_to_subscriptions',
            'action':'string:${object_url}/addSubscription',
            'condition': 'python:member and portal.portal_subscription.getProvider() is not None',
            'permission': ManageSubscriptionContent,
            'category':'document_actions',
            'visible': 1
            },
        {
            'id': 'addGroupSubscription',
            'name': 'action_add_group_subscriptions',
            'action':'string:${object_url}/grouped_subscription_add_form',
            'condition': "python: member and portal.portal_subscription.getProvider() is not None",
            'permission': ManageSubscriptionContent,
            'category':'object',
            'visible': 1
            },
        {
            'id': 'mailing',
            'name': 'action_subscription_mailing',
            'action':'string:${object_url}/subscription_mailing',
            'condition': "python: member and portal.portal_subscription.getProvider() is not None",
            'permission': ManageSubscriptionContent,
            'category':'object',
            'visible': 1
            },
        {
            'id': 'addSubscriptionProvider',
            'name': 'action_add_subscription_provider',
            'action':'string:${object_url}/addSubscriptionProvider',
            'condition': "python: portal.portal_subscription.getShow_actions() and 'subscription_provider' not in object.objectIds()",
            'permission': ManagePortal,
            'category':'folder',
            'visible': 1
            },
        {
            'id': 'addTemplatesProvider',
            'name': 'action_add_templates_provider',
            'action':'string:${object_url}/addTemplatesProvider',
            'condition': "python: portal.portal_subscription.showActions() and 'subscription_templates' not in object.objectIds()",
            'permission': ManagePortal,
            'category':'folder',
            'visible': 1
            },
        )
        
    ai = ActionsInstaller(pf_actions, actions_provider='portal_actions')
    installers.append(ai)

    pf_navtree_props =(
        ('metaTypesNotToList',
         ('SubscriptionTool',
          'SubscriptionProvider',
          'TemplatesProvider',
          )
         ),
        )
    spi = StandardPropertiesInstaller('navtree_properties', pf_navtree_props)
    installers.append(spi)

    sti = ToolInstaller(SubscriptionTool)
    installers.append(sti)

    si = SkinLayersInstaller()
    installers.append(si)

    ci = ConfigletsInstaller(subscription_prefs_configlet)
    installers.append(ci)

    pi = PortletInstaller(('here/portlet_keywords_subscription/macros/portlet',),
                           slot_prop_name='left_slots')
    installers.append(pi)

    pi = PortletInstaller(('here/portlet_subscription/macros/portlet',),
                          slot_prop_name='right_slots')
    installers.append(pi)

    wfi = WorkflowInstaller(workflow_name='subscription_content_workflow',
                            portal_types=('FolderSubscription',
                                          'KeywordsSubscription',
                                          'ContentSubscription',
                                          'ExactSearchSubscription'),
                            old_workflow=None,
                            module_name='Install',
                            function_name='create_subscription_content_workflow',
                            defaultWorkflow=False)
    installers.append(wfi)

    return InstallationRunner(*tuple(installers))

def manageMaintenanceTask(self, action='install'):
    """Add all needed stuff in portal_maintenance"""
    mtool = getToolByName(self, 'portal_maintenance', None)
    if not mtool:
        return
    if action in ('install', 'reinstall'):
        # Plone Catalog update script
        manage_addExternalMethod(mtool.scripts, subscription_script_id,
                                 'Mailing subscriptions',
                                 PROJECTNAME+'.MaintenanceToolbox',
                                 'runMailing')

        try:
            mtool.tasks.manage_addProduct['PloneMaintenance'].addMaintenanceTask(subscription_task_id)
        except:
            pass
        subscription_task = mtool.tasks.subscription_task
        subscription_task.scheduled_minute = '0'
        subscription_task.script_name = subscription_script_id

    else:
        if subscription_task_id in mtool.tasks.objectIds():
            mtool.tasks.manage_delObjects(ids=[subscription_task_id,])
        if subscription_script_id in mtool.scripts.objectIds():
            mtool.scripts.manage_delObjects(ids=[subscription_script_id,])

def install_workflows(self, types, out):
    """Set subscription types outside workflows"""
    wf_tool = getToolByName(self, 'portal_workflow')

    wf_tool.setChainForPortalTypes([type['portal_type'] for type in types], '')

def install_providers(self, out):
    """Instanciate subscription and template providers in the portal root"""
    utool = getToolByName(self, 'portal_url')
    stool = getToolByName(self, 'portal_subscription')
    portal = utool.getPortalObject()

    if not stool.getProvider():
        ob = None
        if not 'subscription_provider' in portal.objectIds():
            portal.invokeFactory('SubscriptionProvider', id='subscription_provider')
        ob = getattr(portal, 'subscription_provider')
        stool.setProvider(ob.UID())

    if not stool.getCatalog():
        manage_addSubscriptionCatalog(portal, 'subscription_catalog', 'Subscription catalog')

    template = stool.getTemplates()
    if not template:
        if not 'subscription_templates' in portal.objectIds():
            portal.invokeFactory('TemplatesProvider', id='subscription_templates')
        template = getattr(portal, 'subscription_templates')
        stool.setTemplates(template.UID())

    if not stool.getFolderTemplate():
        if not 'default_template' in template.objectIds():
            template.invokeFactory('SubscriptionTemplate', id='default_template')
        ob = getattr(template, 'default_template')
        stool.setFolderTemplate(ob.UID())

    if not stool.getContentTemplate():
        if not 'default_template' in template.objectIds():
            template.invokeFactory('SubscriptionTemplate', id='default_template')
        ob = getattr(template, 'default_template')
        stool.setContentTemplate(ob.UID())

    if not stool.getKeywordsTemplate():
        if not 'default_template' in template.objectIds():
            template.invokeFactory('SubscriptionTemplate', id='default_template')
        ob = getattr(template, 'default_template')
        stool.setKeywordsTemplate(ob.UID())

    if not stool.getExactSearchTemplate():
        if not 'default_template' in template.objectIds():
            template.invokeFactory('ExactSearchTemplate', id='default_template')
        ob = getattr(template, 'default_template')
        stool.setExactSearchTemplate(ob.UID())

def install_archetypes_catalogs(self, out):
    """Install Archetypes Catalogs"""
    # We don't want subscription specific content to appear in the
    # portal_catalog. We use our own subscription catalog for that."""
    at=getToolByName(self, 'archetype_tool')
    for meta_type in ("KeywordsSubscription",
                      "ContentSubscription",
                      "FolderSubscription",
                      "ExactSearchSubscription",
                      ):
        at.setCatalogsByType(meta_type,('subscription_catalog',))

    # We don't want the subscription_catalog and the subscribers themselves to be
    # to be cataloged
    for meta_type in ("SubscriptionTemplate",
                      "TemplatesProvider",
                      "SubscriptionProvider",
                      "SubscriptionTool",
                      "UserSubscriber",
                      "AnonymousSubscriber",
                      "GroupSubscriber"):
        at.setCatalogsByType(meta_type,())

def install(self, reinstall=False):
    out = StringIO()

    install_archetypes_catalogs(self, out)

    types = listTypes(PROJECTNAME)
    installTypes(self, out, types,  PROJECTNAME)
    out.write("Successfully installed %s." % PROJECTNAME)

    install_workflows(self, types, out)

    # Always start with the creation of the InstallationContext
    ic = InstallationContext(self, GLOBALS)

    for product in zope_DEPS:
        ic.requiresProduct(product)
    for product in plone_DEPS:
        ic.requiresInstalledProduct(product)

    # Runs the installation and return the report
    out.write(getRunners().install(ic, auto_reorder=True))

    ai=getToolByName(self, 'portal_actionicons')
    try:
        ai.addActionIcon('plone', 'addSubscription', 'subscription_icon.gif', 'AddSubscription')
    except KeyError:
        pass #Duplicate definition!

    if not reinstall:
        manageMaintenanceTask(self, 'install')

    install_providers(self, out)

    provider = getToolByName(self, 'portal_selenium', None)
    if provider:
        # Functional Tests
        action = {'id':PROJECTNAME.lower(),
                  'name':PROJECTNAME,
                  'action':'string:here/get_%s_ftests'%PROJECTNAME.lower(),
                  'condition': '',
                  'permission': 'View',
                  'category':'ftests',
                  'visible': 1}
        provider.addAction(**action)

    out.flush()
    return

def uninstall(self):

    manageMaintenanceTask(self, 'uninstall')

    ai=getToolByName(self, 'portal_actionicons')
    try:
        ai.removeActionIcon('plone', 'addSubscription')
    except KeyError:
        pass #Missing definition!

    # Always start with the creation of the InstallationContext
    ic = InstallationContext(self, GLOBALS)

    # Runs the uninstallation and return the report
    report = getRunners().uninstall(ic, auto_reorder=True)

    return report
