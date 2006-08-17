from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin

from Products.CMFCore.utils import getToolByName
from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod

from Products.rendezView.config import PROJECTNAME, GLOBALS

from StringIO import StringIO


def install(self):
	out = StringIO()
	installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
	install_subskin(self, out, GLOBALS)

	wf_tool = getToolByName(self, 'portal_workflow')
	if not 'participant_workflow' in wf_tool.objectIds():
		wf_tool.manage_addWorkflow('participant_workflow (Participant Workflow)','participant_workflow')

	wf_tool.setChainForPortalTypes(('participant',), chain='participant_workflow')

	out.write("Successfully installed %s." % PROJECTNAME)
	return out.getvalue()
