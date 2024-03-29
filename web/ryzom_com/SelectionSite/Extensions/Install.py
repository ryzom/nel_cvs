from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin
from Products.SelectionSite.config import PROJECTNAME, GLOBALS
from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod
from Products.CMFCore.utils import getToolByName
from StringIO import StringIO

def install(self):
    out = StringIO()
    installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
    install_subskin(self, out, GLOBALS)
    portal = getToolByName(self,'portal_url').getPortalObject()
    manage_addExternalMethod(portal,'FanVisit','Get the Visit',PROJECTNAME + '.FanVisit','FanVisit')
    out.write("Successfully installed %s." % PROJECTNAME)
    return out.getvalue()
