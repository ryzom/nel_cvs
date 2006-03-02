from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin
from Products.Ryzom.config import PROJECTNAME, GLOBALS
from StringIO import StringIO
from Products.CMFCore.utils import getToolByName
from Products.Ryzom import workflow_scripts

def install(self):
    out = StringIO()    
    installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
    install_subskin(self, out, GLOBALS)

    #add workflow

    wf_tool = getToolByName(self, 'portal_workflow')
    if not 'quills_workflow' in wf_tool.objectIds():
       wf_tool.manage_addWorkflow('quills_workflow (Quills Workflow (ryzom))','quills_workflow')
    if not 'plone_workflow' in wf_tool.objectIds():
       wf_tool.manage_addWorkflow('plone_workflow (Default Workflow (ryzom))','plone_workflow')
    if not 'folder_workflow' in wf_tool.objectIds():
       wf_tool.manage_addWorkflow('folder_workflow (Folder Workflow (ryzom))','folder_workflow')
    
    
    #setup workflow script for quills
#    wf = wf_tool.getWorkflowById('quills_workflow')
#    for p in ['moveToArchive', 'moveToWeblogRoot']:
#        if not p in wf.scripts.objectIds():
#            factory = wf.scripts.manage_addProduct['ExternalMethod']
#            factory.manage_addExternalMethod(p, p, 'Ryzom.workflow_scripts', p)

#    wf = wf_tool.getWorkflowById('plone_workflow')
#    if not 'email_notifier' in wf.scripts.onjectIds():
#            factory = wf.scripts.manage_addProduct['script']

      
    out.write("Successfully installed %s." % PROJECTNAME)
    return out.getvalue()
