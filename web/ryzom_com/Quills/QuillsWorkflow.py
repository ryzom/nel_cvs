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
# Quills is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Quills; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##############################################################################

from Products.CMFCore.WorkflowTool import addWorkflowFactory
from Products.DCWorkflow.DCWorkflow import DCWorkflowDefinition

def setupQuills_workflow(wf):
    "..."
    wf.setProperties(title='Quills Workflow')

    for s in ['draft', 'published']:
        wf.states.addState(s)
    for t in ['make_draft', 'publish', 'publish_in_place']:
        wf.transitions.addTransition(t)
    for v in []:
        wf.variables.addVariable(v)
    for l in []:
        wf.worklists.addWorklist(l)
    for p in ('Modify portal content', 'View'):
        wf.addManagedPermission(p)

    ## Initial State
    wf.states.setInitialState('draft')

    ## States initialization
    sdef = wf.states['draft']
    sdef.setProperties(title="", 
            transitions=('publish','publish_in_place'))
    sdef.setPermission('Modify portal content', 0, ['Manager', 'Owner'])
    sdef.setPermission('View', 0, ['Manager', 'Owner'])

    sdef = wf.states['published']
    sdef.setProperties(title="", transitions=('make_draft',))
    sdef.setPermission('Modify portal content', 0, ['Manager', 'Owner'])


    ## Transitions initialization
    tdef = wf.transitions['make_draft']
    tdef.setProperties(title="Member makes content a draft",
                       new_state_id="draft",
                       trigger_type=1,
                       script_name="",
                       after_script_name="moveToWeblogRoot",
                       actbox_name="Revert to Draft",
                       actbox_url="",
                       actbox_category="workflow",
                       props={'guard_expr': 'python: user.has_permission("Manage portal content", here) or user.has_role("Owner", here)' },
                       )

    tdef = wf.transitions['publish']
    tdef.setProperties(title="Reviewer publishes content",
                       new_state_id="published",
                       trigger_type=1,
                       script_name="",
                       after_script_name="moveToArchive",
                       actbox_name="Publish",
                       actbox_url="",
                       actbox_category="workflow",
                       props={'guard_expr': 'python: user.has_permission("Review portal content", here) or user.has_role("Owner", here)' },
                       )

    tdef = wf.transitions['publish_in_place']
    tdef.setProperties(title="Reviewer publishes content without invoking workflow script",
                       new_state_id="published",
                       trigger_type=1,
                       script_name="",
                       after_script_name="",
                       actbox_name="",
                       actbox_url="",
                       actbox_category="",
                       props={'guard_expr': 'python: user.has_permission("Review portal content", here) or user.has_role("Owner", here)' },
                       )

    ## State Variable
    wf.variables.setStateVar('review_state')


def createQuills_workflow(id):
    "..."
    ob = DCWorkflowDefinition(id)
    setupQuills_workflow(ob)
    return ob

def install():
    addWorkflowFactory(createQuills_workflow,
            id='quills_workflow', 
            title='Quills Workflow')
