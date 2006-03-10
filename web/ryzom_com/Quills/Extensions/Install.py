#
# Authors: Brian Skahan <bskahan@etria.com>
#          Tom von Schwerdtner <tvon@etria.com>
#
# Copyright 2004-2005, Etria, LLP
#
# This file is part of Quills
#
# Quills is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Foobar is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Quills; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from Acquisition import aq_base
from StringIO import StringIO

from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin
from Products.CMFCore.utils import getToolByName
from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod

from Products.Quills.config import *
from Products.Quills.properties import *
from Products.Quills import QuillsWorkflow

def install(self):
    out = StringIO()

    portal = getToolByName(self, 'portal_url').getPortalObject()
    # Add the quills_tool
    if not 'quills_tool' in portal.objectIds():
        self.manage_addProduct['Quills'].manage_addTool('Quills Tool', None)

    # Add the site administration interface
    # XXX disabling till we have some config options
    #portal_controlpanel = getToolByName(self, 'portal_controlpanel')
    #portal_controlpanel.registerConfiglets(configlets)

    installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
    install_subskin(self, out, GLOBALS)

    prop_tool = getToolByName(self, 'portal_properties')
    if 'quills_properties' not in prop_tool.objectIds():
        prop_tool.manage_addPropertySheet('quills_properties',
                'Quills Properties')

    quills_props = prop_tool.quills_properties

    for item in props:
        if quills_props.hasProperty(item[0]):
            quills_props._updateProperty(item[0], item[1])
        else:
            quills_props._setProperty(item[0], item[1], item[2])

    # Create an RPCAuth if there is not one already
    installRPCAuth(self)

    # Keep some of our types out of the navtree
    updateNavtreeProperties(self)

    # Setup workflow
    QuillsWorkflow.install()

    wf_tool = getToolByName(self, 'portal_workflow')

    if not 'quills_workflow' in wf_tool.objectIds():
         wf_tool.manage_addWorkflow('quills_workflow (Quills Workflow)',
                                    'quills_workflow')
    wf = wf_tool.getWorkflowById('quills_workflow')
    for p in ['moveToArchive', 'moveToWeblogRoot']:
        if not p in wf.scripts.objectIds():
            factory = wf.scripts.manage_addProduct['ExternalMethod']
            factory.manage_addExternalMethod(p, p, 'Quills.workflow_scripts', p)

    wf_tool.updateRoleMappings()
    print >> out, "Installed Quills Workflow"

    wf_tool.setChainForPortalTypes(('WeblogEntry',), chain='quills_workflow') 

    print >> out, 'Set quills_workflow as default for WeblogEntry'

    # Add external methods for miration
    addMigrationScripts(self)

    print >> out, "Successfully installed %s." % PROJECTNAME
    return out.getvalue()


def installRPCAuth(self):
    if not hasattr(self, 'RPCAuth'):
        try:
            self.manage_addProduct['RPCAuth'].manage_addRPCAuth('RPCAuth')
        except:
            raise "An RPCAuth instance could not be created.  Please make sure RPCAuth is installed correctly."


def updateNavtreeProperties(self):
    ptool = getToolByName(self, 'portal_properties')
    unlisted = ptool.navtree_properties.getProperty('metaTypesNotToList')
    new_unlisted = []

    for item in unlisted:
        new_unlisted.append(item)

    new_unlisted.append('WeblogEntry')
    new_unlisted.append('WeblogArchive')
    new_unlisted.append('WeblogDrafts')
    ptool.navtree_properties._updateProperty('metaTypesNotToList', new_unlisted)


def addMigrationScripts(self):
    portal = getToolByName(self,'portal_url').getPortalObject()
    manage_addExternalMethod(portal,'migrateQuills08to09',
            'Migrate from Quills 0.8 to 0.9',
            PROJECTNAME + '.migration',
            'migrate08to09')


def removeMigrationScripts(self):
    portal = getToolByName(self,'portal_url').getPortalObject()
    for script in ('migrateQuills08to09',):
        if hasattr(aq_base(portal), script):
            portal.manage_delObjects(ids=[script,])


def uninstall(self):
    out = StringIO()

    portal_controlpanel = getToolByName(self, 'portal_controlpanel')
    portal_controlpanel.unregisterApplication('Quills')

    # Remove anything we added for migration
    removeMigrationScripts(self)

    print >> out, "Successfully uninstalled %s." % PROJECTNAME
    return out.getvalue()

