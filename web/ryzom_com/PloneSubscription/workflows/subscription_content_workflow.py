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
Base subscription content workflow.

Use to have the rights roles in allowedRolesAndUsers index.
"""
__version__ = "$Revision: 1.1 $"
# $Source: /mnt/x/wsl/cvsexp3/cvs/code/web/ryzom_com/PloneSubscription/workflows/subscription_content_workflow.py,v $
# $Id: subscription_content_workflow.py,v 1.1 2006/04/03 13:44:37 bernard Exp $
__docformat__ = 'restructuredtext'

from Products.CMFCore.WorkflowTool import addWorkflowFactory
from Products.DCWorkflow.DCWorkflow import DCWorkflowDefinition
from Products.PythonScripts.PythonScript import PythonScript

def setupSubscription_content_workflow(wf):
    """Setups the workflow."""
    wf.setProperties(title='Default Workflow [Plone]')

    for s in ['visible', 'private', 'published']:
        wf.states.addState(s)
    for t in ['retract', 'hide', 'publish', 'show']:
        wf.transitions.addTransition(t)
    for v in ['action', 'review_history', 'actor', 'comments', 'time']:
        wf.variables.addVariable(v)
    for l in []:
        wf.worklists.addWorklist(l)
    for p in ('View', 'PlacelessSubscription: View Content'):
        wf.addManagedPermission(p)

    ## Initial State
    wf.states.setInitialState('private')

    ## States initialization
    sdef = wf.states['private']
    sdef.setProperties(title='Visible and editable only by owner',
                       description='',
                       transitions=('show',))
    sdef.setPermission('View', 0, ['Manager', 'Owner', 'SubscriptionViewer'])
    sdef.setPermission('PlacelessSubscription: View Content', 0, ['Manager', 'Owner', 'SubscriptionViewer'])

    sdef = wf.states['visible']
    sdef.setProperties(title='Visible but not published',
                       description='',
                       transitions=('hide', 'publish'))
    sdef.setPermission('View', 0, ['Manager', 'Member', 'Owner', 'SubscriptionViewer'])
    sdef.setPermission('PlacelessSubscription: View Content', 0, ['Manager', 'Member', 'Owner', 'SubscriptionViewer'])

    sdef = wf.states['published']
    sdef.setProperties(title='Public',
                       description='',
                       transitions=('hide', 'retract'))
    sdef.setPermission('View', 0, ['Anonymous', 'Manager', 'Owner', 'SubscriptionViewer'])
    sdef.setPermission('PlacelessSubscription: View Content', 0, ['Anonymous', 'Manager', 'Owner', 'SubscriptionViewer'])

    ## Transitions initialization
    tdef = wf.transitions['retract']
    tdef.setProperties(title='',
                       description='Member retracts submission',
                       new_state_id='visible',
                       trigger_type=1,
                       script_name='',
                       after_script_name='',
                       actbox_name='Retract',
                       actbox_url='%(content_url)s/content_retract_form',
                       actbox_category='workflow',
                       props={'guard_roles': 'Owner; Manager'},
                       )

    tdef = wf.transitions['hide']
    tdef.setProperties(title='',
                       description='Member makes content private',
                       new_state_id='private',
                       trigger_type=1,
                       script_name='',
                       after_script_name='',
                       actbox_name='Make private',
                       actbox_url='%(content_url)s/content_hide_form',
                       actbox_category='workflow',
                       props={'guard_roles': 'Owner; Manager'},
                       )

    tdef = wf.transitions['publish']
    tdef.setProperties(title='',
                       description='Reviewer publishes content',
                       new_state_id='published',
                       trigger_type=1,
                       script_name='',
                       after_script_name='',
                       actbox_name='Publish',
                       actbox_url='%(content_url)s/content_publish_form',
                       actbox_category='workflow',
                       props={'guard_roles': 'Owner; Manager'},
                       )

    tdef = wf.transitions['show']
    tdef.setProperties(title='',
                       description='Member makes content visible',
                       new_state_id='visible',
                       trigger_type=1,
                       script_name='',
                       after_script_name='',
                       actbox_name='Make visible',
                       actbox_url='%(content_url)s/content_show_form',
                       actbox_category='workflow',
                       props={'guard_roles': 'Owner; Manager'},
                       )

    ## State Variable
    wf.variables.setStateVar('review_state')

    ## Variables initialization
    vdef = wf.variables['action']
    vdef.setProperties(description='The last transition',
                       default_value='',
                       default_expr='transition/getId|nothing',
                       for_catalog=0,
                       for_status=1,
                       update_always=1,
                       props=None)

    vdef = wf.variables['review_history']
    vdef.setProperties(description='Provides access to workflow history',
                       default_value='',
                       default_expr='state_change/getHistory',
                       for_catalog=0,
                       for_status=0,
                       update_always=0,
                       props={'guard_permissions': 'Request review; Review portal content'})

    vdef = wf.variables['actor']
    vdef.setProperties(description='The ID of the user who performed the last transition',
                       default_value='',
                       default_expr='user/getId',
                       for_catalog=0,
                       for_status=1,
                       update_always=1,
                       props=None)

    vdef = wf.variables['comments']
    vdef.setProperties(description='Comments about the last transition',
                       default_value='',
                       default_expr="python:state_change.kwargs.get('comment', '')",
                       for_catalog=0,
                       for_status=1,
                       update_always=1,
                       props=None)

    vdef = wf.variables['time']
    vdef.setProperties(description='Time of the last transition',
                       default_value='',
                       default_expr='state_change/getDateTime',
                       for_catalog=0,
                       for_status=1,
                       update_always=1,
                       props=None)

    ## Worklists Initialization
def createSubscription_content_workflow(id):
    """Create, setup and return the workflow."""
    ob = DCWorkflowDefinition(id)
    setupSubscription_content_workflow(ob)
    return ob

addWorkflowFactory(createSubscription_content_workflow,
                   id='subscription_content_workflow',
                   title='Default Workflow [Plone]')
